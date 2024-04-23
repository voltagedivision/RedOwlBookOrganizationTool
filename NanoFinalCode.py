import easyocr
from fuzzywuzzy import process, fuzz
import cv2
import numpy as np
import os
import time
from datetime import datetime, timedelta
import firebase_admin
from firebase_admin import credentials, firestore
import re
import random
import matplotlib.pyplot as plt
import string

# Set random seed for numpy
np.random.seed(0)

# Set random seed for Python's built-in random module
random.seed(0)

# Global variables
current_sensor_id = 1  # Start with camera 0
book_positions = {}  # Track the highest rank/position assigned to books

# Initialize Firebase Admin SDK
cred = credentials.Certificate("red-owl-book-organization-tool-firebase-adminsdk-hdgjd-5ad51fd0c1.json")
firebase_admin.initialize_app(cred)

# Initialize Firestore client
db = firestore.client()

#print("Connected to Firestore")

# print("Initializing OCR reader...")
# Initialize OCR reader outside the loop
reader = easyocr.Reader(['en'], detector='dbnet18')

# Helper functions for LED mapping
def calculate_led_ranges(start=44, end=926, num_leds=48):
    """
    Calculates the y-coordinate ranges for each LED.
    """
    range_per_led = (end - start) / num_leds
    led_ranges = [(start + i * range_per_led, start + (i + 1) * range_per_led) for i in range(num_leds)]
    return led_ranges

def find_leds_to_turn_on(y_coordinate, led_ranges):
    """
    Determines which LEDs correspond to the given y-coordinate, adjusting for reverse order.
    """
    leds_to_turn_on = []
    # Reversed range to invert LED ordering
    reversed_led_ranges = list(reversed(led_ranges))
    for index, (start, end) in enumerate(reversed_led_ranges):
        if start <= y_coordinate < end:
            # Use reverse indexing to turn on the correct LED group
            leds_to_turn_on.append(len(led_ranges) - 1 - index)
    return leds_to_turn_on

def get_led_group_ids(start_char='0', num_groups=16):
    """
    Generate a list of LED group identifiers starting from 'start_char'.
    The list includes 10 digits and then letters, for a total of 'num_groups' identifiers.
    """
    if start_char.isdigit():
        group_ids = [str(i) for i in range(10)] + list(string.ascii_lowercase)[:num_groups-10]
    else:
        start_index = string.ascii_lowercase.index(start_char.lower())
        group_ids = list(string.ascii_lowercase)[start_index:start_index+num_groups]
    return group_ids[::1]

def map_coordinates_to_leds(detected_titles, led_ranges, current_sensor_id):
    """
    Maps the y-coordinates of detected titles to a single LED group for the specified shelf
    (determined by 'current_sensor_id') and prints which LED group to turn on.
    Adjusts for coordinates outside the range by selecting the nearest LED group at the edges.
    Updates the 'locationGroup' field in the Firestore database for each book.
    """
    # Determine the starting group identifier based on the sensor (shelf) ID
    start_group_id = '0' if current_sensor_id == 0 else 'G'
    led_group_ids = get_led_group_ids(start_group_id, 16)
   
    for title_info in detected_titles:
        top_left = title_info['coordinates'][0]
        bottom_right = title_info['coordinates'][1]
        avg_y = (top_left[1] + bottom_right[1]) / 2
        primary_led = find_leds_to_turn_on(avg_y, led_ranges)

        # Check if primary_led is empty, indicating y-coordinate is outside the LED ranges
        if not primary_led:
            # Determine if the coordinate is below the lowest range or above the highest range
            if avg_y < led_ranges[0][0]:
                # Assign to the first LED group
                led_group_to_turn_on = led_group_ids[-1]
            else:
                # Assign to the last LED group
                led_group_to_turn_on = led_group_ids[0]
        else:
            # Calculate LED group index from the end of the list to flip the order
            primary_led_group = (len(led_ranges) - 1 - primary_led[len(primary_led)//2]) // 3
            primary_led_group = max(0, min(primary_led_group, len(led_group_ids) - 1))
            led_group_to_turn_on = led_group_ids[primary_led_group]

        # Update the database
        books_ref = db.collection("books").document(title_info['title'])  # Assuming the title uniquely identifies a document
        books_ref.update({
            'locationGroup': led_group_to_turn_on
        })
       
        print(f"Shelf {current_sensor_id + 1}, for detected title '{title_info['title']}', turn on LED group: {led_group_to_turn_on}")


# Camera capture and image processing functions

def gstreamer_pipeline(sensor_id=0, capture_width=960, capture_height=720, display_width=960, display_height=720, framerate=30, flip_method=0):
    return (
        f"nvarguscamerasrc sensor-id={sensor_id} ! "
        f"video/x-raw(memory:NVMM), width=(int){capture_width}, height=(int){capture_height}, "
        f"framerate=(fraction){framerate}/1 ! nvvidconv flip-method={flip_method} ! "
        f"video/x-raw, width=(int){display_width}, height=(int){display_height}, format=(string)BGRx ! "
        f"videoconvert ! video/x-raw, format=(string)BGR ! appsink"
    )

def capture_image(sensor_id=0):
    #print(f"Capturing image from Camera {sensor_id}...")
    video_capture = cv2.VideoCapture(gstreamer_pipeline(sensor_id=sensor_id), cv2.CAP_GSTREAMER)
    if video_capture.isOpened():
        try:
            ret_val, frame = video_capture.read()
            if ret_val:
                image_path = f"image_{sensor_id}.jpg"
                cv2.imwrite(image_path, frame)
                return image_path
        finally:
            video_capture.release()
    else:
        print(f"Error: Unable to open camera {sensor_id}")
    return None

def delete_previous_image(image_path):
    if os.path.exists(image_path):
        os.remove(image_path)


def rotate_image(image_path):
    # All points are in format [cols, rows]
    pt_A = [0,0]
    pt_B = [190,720]
    pt_C = [757,719]
    pt_D = [960,0]

    img = cv2.imread(image_path)

    # Here, I have used L2 norm. You can use L1 also.
    width_AD = np.sqrt(((pt_A[0] - pt_D[0]) ** 2) + ((pt_A[1] - pt_D[1]) ** 2))
    width_BC = np.sqrt(((pt_B[0] - pt_C[0]) ** 2) + ((pt_B[1] - pt_C[1]) ** 2))
    maxWidth = max(int(width_AD), int(width_BC))

    height_AB = np.sqrt(((pt_A[0] - pt_B[0]) ** 2) + ((pt_A[1] - pt_B[1]) ** 2))
    height_CD = np.sqrt(((pt_C[0] - pt_D[0]) ** 2) + ((pt_C[1] - pt_D[1]) ** 2))
    maxHeight = max(int(height_AB), int(height_CD))

    input_pts = np.float32([pt_A, pt_B, pt_C, pt_D])
    output_pts = np.float32([[0, 0],
                              [0, maxHeight - 1],
                              [maxWidth - 1, maxHeight - 1],
                              [maxWidth - 1, 0]])

    # Compute the perspective transform M
    M = cv2.getPerspectiveTransform(input_pts,output_pts)

    out = cv2.warpPerspective(img, M, (maxWidth, maxHeight), flags=cv2.INTER_LINEAR)

    # Increasing exposure
    out = cv2.convertScaleAbs(out, alpha=2, beta=10)  # You can adjust alpha and beta values as needed

    cv2.imwrite('transformed.jpg', out)
    image = cv2.imread('transformed.jpg')
    rotated_image = cv2.rotate(image, cv2.ROTATE_90_COUNTERCLOCKWISE)
    rotated_image_path = image_path.split('.')[0] + '_rotated.jpg'
    cv2.imwrite(rotated_image_path, rotated_image)
    return rotated_image_path


def match_books(detected_results, titles, image_path, shelf_number):
    global book_positions
    titles_copy = titles.copy()
    matched_titles = []
    # Load the image for drawing
    image_for_drawing = cv2.imread(image_path)

    for result in detected_results:
        position = result[0]
        text = result[1]
        if text is not None:
            corners = np.array(position).astype(np.int32)
            center_x = int((corners[0][0] + corners[2][0]) / 2)
            center_y = int((corners[0][1] + corners[2][1]) / 2)
            box_width = int((corners[1][0] - corners[0][0]) * 0.5)
            box_height = int((corners[2][1] - corners[1][1]) * 0.5)
            top_left = (max(center_x - box_width // 2, 0), max(center_y - box_height // 2, 0))
            bottom_right = (min(center_x + box_width // 2, image_for_drawing.shape[1]),
                            min(center_y + box_height // 2, image_for_drawing.shape[0]))

            best_match, best_score = process.extractOne(text, titles_copy, scorer=fuzz.token_set_ratio)
            if best_match and best_score > 50:
                cv2.rectangle(image_for_drawing, top_left, bottom_right, (0, 255, 0), 3)
                cv2.line(image_for_drawing, top_left, bottom_right, (255, 0, 0), 2)
                matched_titles.append({
                    'title': best_match,
                    'detected_text': text,
                    'coordinates': (top_left, bottom_right),
                    'shelf': shelf_number
                })
                titles_copy.remove(best_match)

                # Update the database
                current_time = datetime.now()
                formatted_time = current_time.strftime("%m-%d-%Y at %H:%M:%S")
                books_ref = db.collection("books").document(best_match)
                books_ref.update({
                    'lastSeen': formatted_time
                })

        if not titles_copy:
            break

    cv2.imwrite(image_path.replace('.jpg', '_with_adjusted_matches.jpg'), image_for_drawing)

    ranked_titles = {title['title']: (shelf_number, idx+1) for idx, title in enumerate(matched_titles)}
    book_positions.update(ranked_titles)

    ranked_titles = sorted(matched_titles, key=lambda x: x['coordinates'][0][1])

    for idx, match in enumerate(ranked_titles, start=1):
        title = match['title']
        detected_text = match['detected_text']
        coordinates = match['coordinates']
        shelf = match['shelf']
        print(f"Book {idx} on shelf '{shelf}': Detected '{title}' with OCR read line: '{detected_text}'. Coordinates: {coordinates}")
       
    if titles_copy:
        for title in titles_copy:
            print(f"Title '{title}' was not detected.")
    return matched_titles

def get_book_list(shelf_number):
    books_ref = db.collection("books")
    shelf_number = str(shelf_number)
    # Query documents where 'shelfNumber' equals shelf_number and 'currentStatus' is 'available'
    docs = books_ref.where("shelfNumber", "==", shelf_number).where("currentStatus", "==", "available").get()
    titles_list = []
    for doc in docs:
        title = doc.to_dict().get("title")
        if title:
            titles_list.append(title.lower())
    return titles_list

led_ranges = calculate_led_ranges()

try:
    while True:
        current_sensor_id = 1 - current_sensor_id  # Alternate between 0 and 1
        shelf_number = current_sensor_id + 1  # Determine shelf number based on sensor ID
        book_titles = get_book_list(shelf_number)

       # print(book_titles)

        image_path = capture_image(sensor_id=current_sensor_id)
        if image_path:
           # print("Performing OCR with EasyOCR...")
            rotated_image_path = rotate_image(image_path)
            result = reader.readtext(rotated_image_path, paragraph=False, batch_size=1)
            # Define common words to remove
            common_words = set(["the", "of", "and", "a", "in", "to", "on"])
           # print(result)
   # Process each detected result
            processed_results = []
           
            for detection in result:
                # Extract text and confidence from the detection
                text = detection[1]
                confidence = detection[2]
   
   # Convert text to lowercase and remove punctuation
                text = text.lower()
                text = re.sub(r'[^\w\s]', '', text)
   
   # Split text into words and remove common words
                words = text.split()
                words = [word for word in words if word not in common_words]
   
   # Reconstruct text without common words
                processed_text = ' '.join(words)
   
   # Append processed text and confidence to the results
                processed_results.append((detection[0], processed_text, confidence))

            if processed_results:
                matched_titles = match_books(processed_results, book_titles, rotated_image_path, shelf_number)
                map_coordinates_to_leds(matched_titles, led_ranges, current_sensor_id)

            delete_previous_image(image_path)  # Cleanup the original image

            delete_previous_image(rotated_image_path)  # Cleanup the rotated image
           # print("MATCH BOOKS RETURNED... THE END")
        else:
            print(f"Failed to capture image from camera {current_sensor_id}.")

except KeyboardInterrupt:
    print("Exiting...")