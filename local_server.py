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

class TrafficLightState:
    def __init__(self):
        self.timer = 100
        self.is_green = False
        self.duration = [10, 4, 13]  # 0 - red light, 1 - yellow light, 2 - green light
        self.current_index = 0
        self.remaining_time = 0
        self.current_light_1 = "Green"
        self.current_light_2 = "Red"
        self.is_sync_with_web = False
        self.is_auto = False
        self.lock = threading.Lock()

    def get_is_sync_with_web(self):
        with self.lock:
            return self.is_sync_with_web

    def set_is_sync_with_web(self, value):
        with self.lock:
            self.is_sync_with_web = value

    def get_duration(self):
        with self.lock:
            return self.duration

    def set_duration(self, index, value):
        with self.lock:
            self.duration[index] = value

    def get_remaining_time(self):
        with self.lock:
            return self.remaining_time

    def set_remaining_time(self, value):
        with self.lock:
            self.remaining_time = value

    def get_current_light_1(self):
        with self.lock:
            return self.current_light_1

    def set_current_light_1(self, value):
        with self.lock:
            self.current_light_1 = value

    def get_current_light_2(self):
        with self.lock:
            return self.current_light_2

    def set_current_light_2(self, value):
        with self.lock:
            self.current_light_2 = value

    def getAll(self):
        with self.lock:
            return self.current_light_1, self.current_light_2, self.remaining_time, self.duration

    def getState(self):
        with self.lock:
            return self

    def getIsAuto(self):
        with self.lock:
            return self.is_auto

    def setIsAuto(self, value):
        with self.lock:
            self.is_auto = value

state = TrafficLightState()

def handle_change(light_1, light_2):
    data = {
        "light_1": light_1,
        "light_2": light_2
    }
    print(f"Posting data to localhost: {data}")
    try:
        response = requests.post('http://192.168.137.24:5000/postjson', json=data)
        response.raise_for_status()
        print(f"Posted data to localhost: {response.status_code}\n///////////////////////////////////////")
    except requests.exceptions.RequestException as e:
        print(f"Failed to post data: {e}")

def countdown_timer():
    print("Countdown timer started")
    while True:
        temp_light_1 = state.get_current_light_1()
        temp_light_2 = state.get_current_light_2()
        
        print(f"Duration: {state.get_duration()}")
        print(f"Remaining time: {state.get_remaining_time()}")
        timer = 0
        for i in range(3):
            timer += state.get_duration()[i]
    
        timer_red = state.get_duration()[0]
        timer_red_yellow = state.get_duration()[0] + state.get_duration()[1]
        if state.get_remaining_time() < timer_red:
            state.current_index = 0
        elif state.get_remaining_time() < timer_red_yellow:
            state.current_index = 1
        else:
            state.current_index = 2

        print(f"Current index: {state.current_index}")
        
        if state.current_index == 0:
            state.set_current_light_1("Red")
            if state.get_remaining_time() < state.get_duration()[1]:  # Thời gian đèn vàng của đèn 2 là 3s đầu tiên
                state.set_current_light_2("Yellow")
            else:
                state.set_current_light_2("Green")
        else:
            if state.current_index == 1:
                state.set_current_light_1("Yellow")
            else:
                state.set_current_light_1("Green")
            state.set_current_light_2("Red")  # Đèn 2 luôn là đỏ nếu đèn 1 đang xanh hoặc


        # print(f"Remaining time: {state.get_remaining_time()}, Light 1: {state.get_current_light_1()}, Light 2: {state.get_current_light_2()}")
        # if state.get_current_light_1() == "Green":
        #     print("Light 1 green: ", state.get_remaining_time() - state.get_duration()[0] - state.get_duration()[1])
        # elif state.get_current_light_1() == "Yellow":
        #     print("Light 1 yellow: ", state.get_remaining_time() - state.get_duration()[0])
        # else:
        #     print("Light 1 red: ", state.get_remaining_time())

        # if state.get_current_light_2() == "Green":
        #     print("Light 2 green: ", state.get_remaining_time() - state.get_duration()[1])
        # elif state.get_current_light_2() == "Yellow":
        #     print("Light 2 yellow: ", state.get_remaining_time())
        # else:
        #     print("Light 2 red: ", state.get_remaining_time() - state.get_duration()[0])

        current_light_1, current_light_2, remaining_time, duration = state.getAll()

        if current_light_1 == "Green":
            print("Light 1 green: ", remaining_time - duration[0] - duration[1])
        elif current_light_1 == "Yellow":
            print("Light 1 yellow: ", remaining_time - duration[0])
        else:
            print("Light 1 red: ", remaining_time)

        if current_light_2 == "Green":
            print("Light 2 green: ", remaining_time - duration[1])
        elif current_light_2 == "Yellow":
            print("Light 2 yellow: ", remaining_time)
        else:
            print("Light 2 red: ", remaining_time - duration[0])
        




        print("is_sync_with_web: ", state.get_is_sync_with_web())

        if temp_light_1 != state.get_current_light_1() or temp_light_2 != state.get_current_light_2():
            if not state.get_is_sync_with_web():
                handle_change(state.get_current_light_1(), state.get_current_light_2())
        state.set_remaining_time(state.get_remaining_time() - 1)

        if state.get_remaining_time() < 0:
            state.set_remaining_time(timer)
        time.sleep(1)

def set_timer1_green(green_time):
    state.set_duration(2, green_time)
    print(f"Set timer1 green: Updated duration[2] to {green_time}")

def set_timer2_green(green_time):
    state.set_duration(0, green_time)
    print(f"Set timer2 green: Updated duration[0] to {green_time}")

def listener(event):
    if event.data:
        # print(f"Need to fetch green 1: {event.data}")
        time.sleep(1)  # Wait for 1 second
        green_time_1_ref = db.reference('traffic_system/intersections/main_intersection/lanes/1/green_time')
        green_time_1 = green_time_1_ref.get()
        set_timer1_green(green_time_1)
    else:
        # print(f"Need fetch green 2: {event.data}")
        time.sleep(1)
        green_time_2_ref = db.reference('traffic_system/intersections/main_intersection/lanes/2/green_time')
        green_time_2 = green_time_2_ref.get()
        set_timer2_green(green_time_2)

def listener_1(event):
    print(f"Change detected in light 1: {event.data}")
    light_1 = event.data
    light_2 = db.reference('traffic_system/intersections/main_intersection/color/2/light_color').get()
    handle_change(light_1, light_2)

def listener_2(event):
    print(f"Change detected in light 2: {event.data}")
    light_2 = event.data
    light_1 = db.reference('traffic_system/intersections/main_intersection/color/1/light_color').get()
    handle_change(light_1, light_2)

def start_listeners_local_server_2():
    light_1_ref = db.reference('traffic_system/intersections/main_intersection/color/1/light_color')
    light_2_ref = db.reference('traffic_system/intersections/main_intersection/color/2/light_color')
    light_1_ref.listen(listener_1)
    light_2_ref.listen(listener_2)

# def fetch_data():
#     lanes_ref = db.reference('traffic_system/intersections/main_intersection/lanes')
#     lanes_data = lanes_ref.get()
#     try:
#         is_green_1 = lanes_data[1]['is_green']
#         is_green_2 = lanes_data[2]['is_green']
#         green_time_1 = lanes_data[1]['green_time']
#         green_time_2 = lanes_data[2]['green_time']
#         remaining_time_1 = lanes_data[1]['remaining_time']
#         remaining_time_2 = lanes_data[2]['remaining_time']

#         print(f"Data updated: {is_green_1}, {is_green_2}, {green_time_1}, {green_time_2}, {remaining_time_1}, {remaining_time_2}")

#     except (IndexError, TypeError) as e:
#         print(f"Failed to fetch data: {e}")


def listenerAuto(event):
    state.setIsAuto(event.data)
        

def start_listeners_local_server_1():
    is_green_1_ref = db.reference('traffic_system/intersections/main_intersection/lanes/1/is_green')
    is_green_1_ref.listen(listener)

    is_auto_ref = db.reference('traffic_system/intersections/main_intersection/isAuto')
    is_auto_ref.listen(listenerAuto)



    green_time_1_ref = db.reference('traffic_system/intersections/main_intersection/lanes/1/green_time')
    green_time_1_ref.listen(listenerGreenTimmer1)

    green_time_2_ref = db.reference('traffic_system/intersections/main_intersection/lanes/2/green_time')
    green_time_2_ref.listen(listenerGreenTimmer2)

def listenerGreenTimmer1(event):
    time.sleep(1)
    is_auto_ref = db.reference('traffic_system/intersections/main_intersection/isAuto')
    is_auto = is_auto_ref.get()
    print("Listener green timer 1 auto: ", is_auto)
    if not is_auto:
        print("TIMER 1: ", event.data)
        green_time_1 = event.data
        set_timer1_green(green_time_1)

def listenerGreenTimmer2(event):
    time.sleep(1)
    is_auto_ref = db.reference('traffic_system/intersections/main_intersection/isAuto')
    is_auto = is_auto_ref.get()
    if not is_auto:
        print("TIMER 2: ", event.data)
        green_time_2 = event.data
        set_timer2_green(green_time_2)

def sync_listener(event):
    print(f"123123 Sync with web: {event.data}")
    state.set_is_sync_with_web(event.data)
    if event.data:
        print("Switching to local server 2")
        start_listeners_local_server_2()
    else:
        print("Switching to local server 1")
        start_listeners_local_server_1()

def run_sync_service():
    sync_ref = db.reference('traffic_system/intersections/main_intersection/syncWithRealDevice')
    sync_ref.listen(sync_listener)
    print("Listening for syncWithRealDevice changes. Press Ctrl+C to stop.")

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("Server stopping...")

def main_2():
    # Start the countdown timer in a separate thread
    print("Starting countdown timer...")
    threading.Thread(target=countdown_timer).start()

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("Server stopping...")
    

if __name__ == "__main__":
    threading.Thread(target=run_sync_service).start()
    threading.Thread(target=main_2).start()