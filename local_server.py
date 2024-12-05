import firebase_admin
from firebase_admin import credentials, db
import requests
import json
import threading
import time

# Initialize the app with a service account, granting admin privileges
cred = credentials.Certificate(r"credential\smart-traffic-light-03-firebase-adminsdk-mzf6v-45aa726e71.json")
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://smart-traffic-light-03-default-rtdb.asia-southeast1.firebasedatabase.app/'
})

def fetch_data():
    lanes_ref = db.reference('traffic_system/intersections/main_intersection/lanes')
    lanes_data = lanes_ref.get()
    try:
        response = requests.post('http://localhost:80', json=lanes_data)
        response.raise_for_status()
        print(f"Posted data to localhost: {response.status_code}")
    except requests.exceptions.RequestException as e:
        print(f"Failed to post data: {e}")

def listener(event):
   fetch_data()

def listenerSync(event):
    if event.data:
        print(f"Change detected in data: {event.data}")
        fetch_data()

def start_listeners():
    green_time_1_ref = db.reference('traffic_system/intersections/main_intersection/lanes/1/green_time')
    green_time_2_ref = db.reference('traffic_system/intersections/main_intersection/lanes/2/green_time')
    green_time_1_ref.listen(listener)
    green_time_2_ref.listen(listener)
    is_green_1_ref = db.reference('traffic_system/intersections/main_intersection/lanes/1/is_green')
    is_green_2_ref = db.reference('traffic_system/intersections/main_intersection/lanes/2/is_green')
    is_green_1_ref.listen(listener)
    is_green_2_ref.listen(listener)

    need_sync_ref = db.reference('traffic_system/intersections/main_intersection/needSync')
    need_sync_ref.listen(listenerSync)

def main():
    listener_thread = threading.Thread(target=start_listeners)
    listener_thread.start()
    print("Server started.")

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("Server stopping.")
        listener_thread.join()
        print("Server stopped.")

if __name__ == "__main__":
    main()