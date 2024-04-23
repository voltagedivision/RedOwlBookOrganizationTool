import time
from datetime import datetime, timedelta
import firebase_admin
from firebase_admin import credentials, firestore
from google.cloud.firestore_v1.base_query import FieldFilter
import serial

# Initialize Firebase Admin SDK
cred = credentials.Certificate("/home/pi/red-owl-book-organization-tool-firebase-adminsdk-hdgjd-5ad51fd0c1.json")  # Update the path to your credentials file
firebase_admin.initialize_app(cred)

# Initialize Firestore client
db = firestore.client()

print("Connected to Firestore")

# Open the serial ports (adjust the serial device paths as per your Raspberry Pi setup)
ser1 = serial.Serial('/dev/rfcomm0', 9600, timeout=1)  # Update to the device name for outgoing communications
print("Outgoing port opened")

time.sleep(5)

# Reference to the books collection
books_ref = db.collection("books")

def check_and_update_missing_books():
    # Fetch current time
    current_time = datetime.now()
    # Query all books from the collection
    all_books = books_ref.stream()
   
    for doc in all_books:
        book_data = doc.to_dict()
        last_seen_str = book_data.get("lastSeen")
        if last_seen_str:
            # Parse the lastSeen string to datetime object
            last_seen_time = datetime.strptime(last_seen_str, "%m-%d-%Y at %H:%M:%S")
            # Check if the difference is more than 5 minutes
            if (current_time - last_seen_time) > timedelta(minutes=5):
                # Update the status to "missing"
                doc.reference.update({"currentStatus": "missing"})
                print(f"Book '{book_data.get('title')}' marked as missing.")

try:
    while True:
        time.sleep(5)
        # Check and update missing books
        check_and_update_missing_books()
       
        # Query documents where ledFindFlag is equal to 1
        query = books_ref.where("ledFindFlag", "==", 1).stream()

        # Flag to track if at least one document with ledFindFlag == 1 is found
        flag_triggered = False

        for doc in query:
            book_data = doc.to_dict()
            # Extract necessary information from the book document
            book_title = book_data.get("title")
            print(f"LED find flag for book '{book_title}' is set to 1")
            location_data = book_data.get("locationGroup")
            serial_data = str(location_data)  # Convert location_data to a string
            print("Sending data:", serial_data)
            ser1.flush()
            ser1.write(serial_data.encode())
            time.sleep(1)
            flag_triggered = True  # Set flag to True if at least one document is found

            # Update the Firestore document to set ledFindFlag back to 0
            doc.reference.update({"ledFindFlag": 0})
            print(f"LED find flag for book '{book_title}' set back to 0")

        if flag_triggered:
            print("LED find flag triggered for at least one book")
        else:
            print("No LED find flag triggered")

except KeyboardInterrupt:
    print("Closing serial connection.")

finally:
    ser1.close()