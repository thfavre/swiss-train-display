#!/usr/bin/env python3
"""
Swiss Train Display Simulator - 3 Button Interface
Controls: u=up, d=down, o=ok, U=long_up, D=long_down, O=long_ok
"""

import requests
import time
from datetime import datetime
import os
import sys
import select
import threading

# ANSI color codes
class Colors:
    RESET = '\033[0m'
    BOLD = '\033[1m'
    RED = '\033[91m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    MAGENTA = '\033[95m'
    CYAN = '\033[96m'
    STRIKETHROUGH = '\033[9m'
    BG_WHITE = '\033[47m'
    BLACK = '\033[30m'

def clear_screen():
    os.system('clear' if os.name != 'nt' else 'cls')

def get_connections(from_station, to_station, limit=4):
    try:
        url = "http://transport.opendata.ch/v1/connections"
        params = {'from': from_station, 'to': to_station, 'limit': limit}
        response = requests.get(url, params=params, timeout=10)
        response.raise_for_status()
        return response.json()
    except Exception as e:
        print(f"{Colors.RED}Error: {e}{Colors.RESET}")
        return None

def calculate_minutes_until(departure_time):
    try:
        dep_time = datetime.fromisoformat(departure_time.replace('Z', '+00:00'))
        now = datetime.now(dep_time.tzinfo)
        delta = dep_time - now
        return int(delta.total_seconds() / 60)
    except:
        return 0

def format_time(time_str):
    try:
        dt = datetime.fromisoformat(time_str.replace('Z', '+00:00'))
        return dt.strftime("%H:%M")
    except:
        return time_str

def format_delay(scheduled, prognosis):
    if not prognosis:
        return 0, ""
    try:
        sched = datetime.fromisoformat(scheduled.replace('Z', '+00:00'))
        prog = datetime.fromisoformat(prognosis.replace('Z', '+00:00'))
        delay_mins = int((prog - sched).total_seconds() / 60)
        if delay_mins > 0:
            return delay_mins, f"+{delay_mins}'"
        return 0, ""
    except:
        return 0, ""

def display_connection(conn, is_browsing=False):
    """Display a connection with all details"""
    from_stop = conn['from']
    to_stop = conn['to']
    
    scheduled_dep = from_stop['departure']
    prognosis_dep = from_stop.get('prognosis', {}).get('departure')
    platform = from_stop.get('platform', '?')
    
    delay_mins, delay_str = format_delay(scheduled_dep, prognosis_dep)
    
    sections = conn.get('sections', [])
    is_cancelled = len(sections) == 0 or sections[0].get('journey') is None
    
    print(f"\n{Colors.BOLD}{Colors.CYAN}╔══════════════════════════════════════════════════╗{Colors.RESET}")
    print(f"{Colors.BOLD}{Colors.CYAN}║{Colors.RESET}  {from_stop['station']['name']} → {to_stop['station']['name']}")
    print(f"{Colors.BOLD}{Colors.CYAN}╚══════════════════════════════════════════════════╝{Colors.RESET}\n")
    
    if is_cancelled:
        print(f"{Colors.RED}{Colors.BOLD}{Colors.STRIKETHROUGH}  ❌ TRAIN CANCELLED{Colors.RESET}\n")
        print(f"  {Colors.STRIKETHROUGH}Departure: {format_time(scheduled_dep)}{Colors.RESET}")
        print(f"  {Colors.STRIKETHROUGH}Platform: {platform}{Colors.RESET}")
        return
    
    # Show departure time
    if is_browsing:
        if delay_mins > 0:
            print(f"  🕐 Departure: {Colors.STRIKETHROUGH}{format_time(scheduled_dep)}{Colors.RESET} "
                  f"{Colors.RED}{Colors.BOLD}{format_time(prognosis_dep)}{Colors.RESET} {Colors.RED}({delay_str}){Colors.RESET}")
        else:
            print(f"  🕐 Departure: {Colors.GREEN}{Colors.BOLD}{format_time(scheduled_dep)}{Colors.RESET}")
    else:
        mins_until = calculate_minutes_until(prognosis_dep if prognosis_dep else scheduled_dep)
        if delay_mins > 0:
            original_mins = calculate_minutes_until(scheduled_dep)
            print(f"  ⏱️  Departs in: {Colors.STRIKETHROUGH}{original_mins} min{Colors.RESET} "
                  f"{Colors.RED}{Colors.BOLD}{mins_until} min{Colors.RESET} {Colors.RED}({delay_str}){Colors.RESET}")
        else:
            if mins_until <= 0:
                print(f"  ⏱️  Departs in: {Colors.YELLOW}{Colors.BOLD}NOW!{Colors.RESET}")
            elif mins_until <= 5:
                print(f"  ⏱️  Departs in: {Colors.YELLOW}{Colors.BOLD}{mins_until} min{Colors.RESET}")
            else:
                print(f"  ⏱️  Departs in: {Colors.GREEN}{Colors.BOLD}{mins_until} min{Colors.RESET}")
    
    # Platform
    prognosis_platform = from_stop.get('prognosis', {}).get('platform')
    if prognosis_platform and prognosis_platform != platform:
        print(f"  🚉 Platform: {Colors.STRIKETHROUGH}{platform}{Colors.RESET} "
              f"{Colors.YELLOW}{Colors.BOLD}{prognosis_platform}{Colors.RESET} {Colors.YELLOW}⚠️  CHANGED!{Colors.RESET}")
    else:
        print(f"  🚉 Platform: {Colors.BOLD}{platform}{Colors.RESET}")
    
    # Arrival
    print(f"  📍 Arrival: {Colors.BOLD}{format_time(to_stop['arrival'])}{Colors.RESET}")
    
    # Duration
    duration = conn.get('duration', '')
    if duration:
        parts = duration.split(':')
        if len(parts) >= 2:
            hours = int(parts[-3].split('d')[-1]) if 'd' in parts[-3] else 0
            minutes = int(parts[-2])
            duration_str = f"{hours}h {minutes}min" if hours > 0 else f"{minutes} min"
            print(f"  ⏱️  Duration: {duration_str}")
    
    # Journey details
    if sections:
        print(f"\n  {Colors.BOLD}🚆 Journey:{Colors.RESET}")
        for i, section in enumerate(sections, 1):
            journey = section.get('journey')
            if journey:
                train_name = f"{journey.get('category', '')} {journey.get('number', '')}".strip()
                to_dest = journey.get('to', 'Unknown')
                print(f"     {Colors.CYAN}{train_name}{Colors.RESET} → {to_dest}")
                
                # Occupancy
                capacity2 = journey.get('capacity2nd')
                if capacity2 is not None and capacity2 != '-1':
                    try:
                        cap_int = int(capacity2)
                        if cap_int >= 2:
                            print(f"     {Colors.RED}🔴 High occupancy{Colors.RESET}")
                        elif cap_int >= 1:
                            print(f"     {Colors.YELLOW}🟡 Medium occupancy{Colors.RESET}")
                        else:
                            print(f"     {Colors.GREEN}🟢 Low occupancy{Colors.RESET}")
                    except (ValueError, TypeError):
                        pass  # Skip if capacity data is invalid

def display_multiple_destinations(from_station, destinations, cached_data=None):
    """Display trains from one station to multiple destinations with cached data"""
    print(f"\n{Colors.BOLD}{Colors.CYAN}╔══════════════════════════════════════════════════╗{Colors.RESET}")
    print(f"{Colors.BOLD}{Colors.CYAN}║{Colors.RESET}  From: {from_station}")
    print(f"{Colors.BOLD}{Colors.CYAN}╚══════════════════════════════════════════════════╝{Colors.RESET}\n")

    if cached_data is None:
        cached_data = {}

    for dest in destinations:
        # Use cached data if available, otherwise show loading
        if dest in cached_data and cached_data[dest]:
            conn = cached_data[dest]
            from_stop = conn['from']

            scheduled_dep = from_stop['departure']
            prognosis_dep = from_stop.get('prognosis', {}).get('departure')
            delay_mins, delay_str = format_delay(scheduled_dep, prognosis_dep)
            platform = from_stop.get('platform', '?')

            sections = conn.get('sections', [])
            is_cancelled = len(sections) == 0 or sections[0].get('journey') is None

            if is_cancelled:
                print(f"  → {dest:20s} {Colors.RED}{Colors.STRIKETHROUGH}CANCELLED{Colors.RESET}")
            else:
                if delay_mins > 0:
                    time_str = f"{Colors.STRIKETHROUGH}{format_time(scheduled_dep)}{Colors.RESET} {Colors.RED}{format_time(prognosis_dep)}{Colors.RESET}"
                    delay_display = f"{Colors.RED}{delay_str}{Colors.RESET}"
                else:
                    time_str = f"{Colors.GREEN}{format_time(scheduled_dep)}{Colors.RESET}"
                    delay_display = ""

                print(f"  🚆 {dest:18s} {time_str} Pl.{platform:2s} {delay_display}")
        else:
            print(f"  → {dest:20s} {Colors.YELLOW}Loading...{Colors.RESET}")

def display_multiple_routes(routes, cached_data=None):
    """Display multiple routes with cached data"""
    print(f"\n{Colors.BOLD}{Colors.CYAN}╔══════════════════════════════════════════════════╗{Colors.RESET}")
    print(f"{Colors.BOLD}{Colors.CYAN}║{Colors.RESET}  Multiple Routes Overview")
    print(f"{Colors.BOLD}{Colors.CYAN}╚══════════════════════════════════════════════════╝{Colors.RESET}\n")

    if cached_data is None:
        cached_data = {}

    for from_station, to_station in routes:
        route_key = (from_station, to_station)

        # Use cached data if available
        if route_key in cached_data and cached_data[route_key]:
            conn = cached_data[route_key]
            from_stop = conn['from']

            scheduled_dep = from_stop['departure']
            prognosis_dep = from_stop.get('prognosis', {}).get('departure')
            delay_mins, delay_str = format_delay(scheduled_dep, prognosis_dep)

            sections = conn.get('sections', [])
            is_cancelled = len(sections) == 0 or sections[0].get('journey') is None

            route_name = f"{from_station[:12]} → {to_station[:12]}"

            if is_cancelled:
                print(f"  🚆 {route_name:28s} {Colors.RED}{Colors.STRIKETHROUGH}CANCELLED{Colors.RESET}")
            else:
                if delay_mins > 0:
                    time_str = f"{Colors.STRIKETHROUGH}{format_time(scheduled_dep)}{Colors.RESET} {Colors.RED}{format_time(prognosis_dep)} {delay_str}{Colors.RESET}"
                else:
                    time_str = f"{Colors.GREEN}{format_time(scheduled_dep)}{Colors.RESET}"

                print(f"  🚆 {route_name:28s} {time_str}")
        else:
            route_name = f"{from_station[:12]} → {to_station[:12]}"
            print(f"  🚆 {route_name:28s} {Colors.YELLOW}Loading...{Colors.RESET}")

def display_clock():
    """Clock display"""
    now = datetime.now()
    print(f"\n\n")
    print(f"        {Colors.BOLD}{Colors.CYAN}╔════════════════════════╗{Colors.RESET}")
    print(f"        {Colors.BOLD}{Colors.CYAN}║{Colors.RESET}                        {Colors.BOLD}{Colors.CYAN}║{Colors.RESET}")
    print(f"        {Colors.BOLD}{Colors.CYAN}║{Colors.RESET}   🕐 {Colors.BOLD}{Colors.GREEN}{now.strftime('%H:%M:%S')}{Colors.RESET}        {Colors.BOLD}{Colors.CYAN}║{Colors.RESET}")
    print(f"        {Colors.BOLD}{Colors.CYAN}║{Colors.RESET}   {now.strftime('%A'):20s}{Colors.BOLD}{Colors.CYAN}║{Colors.RESET}")
    print(f"        {Colors.BOLD}{Colors.CYAN}║{Colors.RESET}   {now.strftime('%d %B %Y'):20s}{Colors.BOLD}{Colors.CYAN}║{Colors.RESET}")
    print(f"        {Colors.BOLD}{Colors.CYAN}║{Colors.RESET}                        {Colors.BOLD}{Colors.CYAN}║{Colors.RESET}")
    print(f"        {Colors.BOLD}{Colors.CYAN}╚════════════════════════╝{Colors.RESET}")

class NumberPicker:
    """Number picker navigable with 3 buttons - for seconds/intervals"""
    def __init__(self, prompt="Enter value:", initial_value=10, min_value=5, max_value=999):
        self.prompt = prompt
        self.value = initial_value
        self.min_value = min_value
        self.max_value = max_value

    def display(self):
        """Display the number picker"""
        clear_screen()
        print(f"\n{Colors.BOLD}{Colors.CYAN}{'═' * 52}{Colors.RESET}")
        print(f"{Colors.BOLD}{Colors.CYAN}  {self.prompt}{Colors.RESET}")
        print(f"{Colors.BOLD}{Colors.CYAN}{'═' * 52}{Colors.RESET}\n")
        print(f"  Current value: {Colors.GREEN}{Colors.BOLD}{self.value}{Colors.RESET}\n")
        print(f"{Colors.CYAN}{'─' * 52}{Colors.RESET}")
        print(f"{Colors.YELLOW}[u] +1  [d] -1  [U] +20  [D] -20  [o] OK{Colors.RESET}")

    def handle_input(self, key):
        """Handle button press"""
        if key == 'u':  # Up by 1
            self.value = min(self.max_value, self.value + 1)
        elif key == 'd':  # Down by 1
            self.value = max(self.min_value, self.value - 1)
        elif key == 'U':  # Long up = +20
            self.value = min(self.max_value, self.value + 20)
        elif key == 'D':  # Long down = -20
            self.value = max(self.min_value, self.value - 20)
        elif key in ['o', 'O']:  # OK
            return True, self.value
        return False, self.value

    def get_value(self):
        """Run the picker and get user input"""
        while True:
            self.display()
            key = input().strip()

            done, value = self.handle_input(key)
            if done:
                return value

class OnScreenKeyboard:
    """On-screen keyboard navigable with 3 buttons"""
    def __init__(self, prompt="Enter text:", numeric_only=False):
        self.prompt = prompt
        self.numeric_only = numeric_only

        if numeric_only:
            # Numeric keyboard is now just for entering station names with numbers
            self.keys = [
                ['1', '2', '3'],
                ['4', '5', '6'],
                ['7', '8', '9'],
                ['0', 'DEL', 'OK']
            ]
        else:
            self.keys = [
                ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I'],
                ['J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R'],
                ['S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' '],
                ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'],
                ['j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r'],
                ['s', 't', 'u', 'v', 'w', 'x', 'y', 'z', ','],
                ['0', '1', '2', '3', '4', '5', '6', '7', '8'],
                ['9', '-', 'DEL', 'CLEAR', 'OK', '', '', '', '']
            ]

        self.row = 0
        self.col = 0
        self.text = ""

    def display(self):
        """Display the keyboard"""
        clear_screen()
        print(f"\n{Colors.BOLD}{Colors.CYAN}{'═' * 52}{Colors.RESET}")
        print(f"{Colors.BOLD}{Colors.CYAN}  {self.prompt}{Colors.RESET}")
        print(f"{Colors.BOLD}{Colors.CYAN}{'═' * 52}{Colors.RESET}\n")
        print(f"  Input: {Colors.GREEN}{Colors.BOLD}{self.text}_  {Colors.RESET}\n")

        # Display keyboard
        for r, row in enumerate(self.keys):
            line = "  "
            for c, key in enumerate(row):
                if key:
                    if r == self.row and c == self.col:
                        # Highlighted key
                        line += f"{Colors.BG_WHITE}{Colors.BLACK}{Colors.BOLD} {key:5s} {Colors.RESET} "
                    else:
                        line += f"{Colors.CYAN}[{key:5s}]{Colors.RESET} "
                else:
                    line += "        "
            print(line)

        print(f"\n{Colors.CYAN}{'─' * 52}{Colors.RESET}")
        print(f"{Colors.YELLOW}[U/u] Up  [D/d] Down  [O/o] Select{Colors.RESET}")

    def handle_input(self, key):
        """Handle button press"""
        if key in ['u', 'U']:  # Up
            self.row = (self.row - 1) % len(self.keys)
            # Skip empty cells
            while self.col < len(self.keys[self.row]) and not self.keys[self.row][self.col]:
                self.col = max(0, self.col - 1)

        elif key in ['d', 'D']:  # Down
            self.row = (self.row + 1) % len(self.keys)
            # Skip empty cells
            while self.col < len(self.keys[self.row]) and not self.keys[self.row][self.col]:
                self.col = max(0, self.col - 1)

        elif key in ['o', 'O']:  # OK
            if self.col < len(self.keys[self.row]):
                selected = self.keys[self.row][self.col]

                if selected == 'OK':
                    return True, self.text
                elif selected == 'DEL':
                    self.text = self.text[:-1]
                elif selected == 'CLEAR':
                    self.text = ""
                elif selected:
                    self.text += selected

            # Move to next key after selection
            self.col = (self.col + 1) % len([k for k in self.keys[self.row] if k])
            if self.col >= len(self.keys[self.row]):
                self.col = 0
                self.row = (self.row + 1) % len(self.keys)

        return False, self.text

    def get_input(self):
        """Run the keyboard and get user input"""
        while True:
            self.display()
            key = input().strip()

            done, text = self.handle_input(key)
            if done:
                return text

class Menu:
    """Menu system navigable with 3 buttons"""
    def __init__(self, title, items):
        self.title = title
        self.items = items  # List of (label, value) tuples
        self.selected = 0

    def display(self):
        """Display the menu"""
        clear_screen()
        print(f"\n{Colors.BOLD}{Colors.CYAN}{'═' * 52}{Colors.RESET}")
        print(f"{Colors.BOLD}{Colors.CYAN}  {self.title}{Colors.RESET}")
        print(f"{Colors.BOLD}{Colors.CYAN}{'═' * 52}{Colors.RESET}\n")

        for i, (label, _) in enumerate(self.items):
            if i == self.selected:
                print(f"  {Colors.BG_WHITE}{Colors.BLACK}{Colors.BOLD}▶ {label:44s}{Colors.RESET}")
            else:
                print(f"  {Colors.CYAN}  {label}{Colors.RESET}")

        print(f"\n{Colors.CYAN}{'─' * 52}{Colors.RESET}")
        print(f"{Colors.YELLOW}[U/u] Up  [D/d] Down  [O/o] Select{Colors.RESET}")

    def handle_input(self, key):
        """Handle button press"""
        if key in ['u', 'U']:  # Up
            self.selected = (self.selected - 1) % len(self.items)
        elif key in ['d', 'D']:  # Down
            self.selected = (self.selected + 1) % len(self.items)
        elif key in ['o', 'O']:  # OK
            return True, self.items[self.selected][1]
        return False, None

    def get_selection(self):
        """Run the menu and get user selection"""
        while True:
            self.display()
            key = input().strip()

            done, value = self.handle_input(key)
            if done:
                return value

class TrainDisplay:
    def __init__(self):
        self.current_preset = 1
        self.connection_index = 0
        self.is_browsing = False
        self.all_connections = []
        self.last_update = 0
        self.update_interval = 60  # Can be set to 0 to disable auto-refresh
        self.running = True
        self.auto_mode = True

        # Auto-loop configuration
        self.loop_enabled = False
        self.loop_presets = [1, 2, 3, 4]  # Which presets to loop through
        self.loop_interval = 10  # Seconds to show each preset
        self.loop_last_switch = time.time()
        self.loop_current_index = 0

        # Background refresh cache with timestamps
        self.cache_preset1 = []  # Cached connections for preset 1
        self.cache_preset1_timestamp = 0
        self.cache_destinations = {}  # {dest: connection_data}
        self.cache_destinations_timestamp = {}  # {dest: timestamp}
        self.cache_routes = {}  # {(from, to): connection_data}
        self.cache_routes_timestamp = {}  # {(from, to): timestamp}
        self.cache_lock = threading.Lock()
        self.refresh_thread = None
        self.is_refreshing = False
        self.preset_last_refresh = {1: 0, 2: 0, 3: 0, 4: 0}  # Track last refresh per preset

        # Preset configurations - CUSTOMIZE THESE!
        self.presets = {
            1: {'name': 'Single Route', 'from': 'Lausanne', 'to': 'Genève'},
            2: {'name': 'Multiple Dest', 'from': 'Lausanne',
                'destinations': ['Genève', 'Bern', 'Zürich', 'Basel']},
            3: {'name': 'Multiple Routes', 'routes': [
                ('Lausanne', 'Genève'),
                ('Bern', 'Zürich'),
                ('Basel', 'Luzern'),
                ('Zürich', 'St. Gallen')
            ]},
            4: {'name': 'Clock'}
        }
    
    def configure_preset(self, preset_num):
        """Configure a specific preset using 3-button interface"""
        preset = self.presets[preset_num]

        if preset_num == 1:
            # Configure single route
            menu = Menu(f"Configure Preset {preset_num}", [
                (f"From: {preset['from']}", 'from'),
                (f"To: {preset['to']}", 'to'),
                ("Back", 'back')
            ])
            action = menu.get_selection()

            if action == 'from':
                keyboard = OnScreenKeyboard("Enter FROM station:")
                preset['from'] = keyboard.get_input() or preset['from']
                self.show_message(f"From station set to: {preset['from']}")

            elif action == 'to':
                keyboard = OnScreenKeyboard("Enter TO station:")
                preset['to'] = keyboard.get_input() or preset['to']
                self.show_message(f"To station set to: {preset['to']}")

        elif preset_num == 2:
            # Configure multiple destinations
            menu = Menu(f"Configure Preset {preset_num}", [
                (f"From: {preset['from']}", 'from'),
                ("Edit destinations", 'destinations'),
                ("Back", 'back')
            ])
            action = menu.get_selection()

            if action == 'from':
                keyboard = OnScreenKeyboard("Enter FROM station:")
                preset['from'] = keyboard.get_input() or preset['from']
                self.show_message(f"From station set to: {preset['from']}")

            elif action == 'destinations':
                # Show menu to add/edit destinations
                dest_menu = Menu("Destinations", [
                    (f"{i+1}. {dest}", i) for i, dest in enumerate(preset['destinations'])
                ] + [("Add new destination", 'add'), ("Back", 'back')])

                dest_action = dest_menu.get_selection()
                if dest_action == 'add':
                    keyboard = OnScreenKeyboard("Enter destination:")
                    new_dest = keyboard.get_input()
                    if new_dest:
                        preset['destinations'].append(new_dest)
                        self.show_message(f"Added: {new_dest}")
                elif isinstance(dest_action, int):
                    keyboard = OnScreenKeyboard(f"Edit destination {dest_action+1}:")
                    keyboard.text = preset['destinations'][dest_action]
                    preset['destinations'][dest_action] = keyboard.get_input()
                    self.show_message(f"Updated destination")

        elif preset_num == 3:
            # Configure multiple routes
            menu = Menu(f"Configure Preset {preset_num}", [
                (f"{i+1}. {from_st} → {to_st}", i) for i, (from_st, to_st) in enumerate(preset['routes'])
            ] + [("Add new route", 'add'), ("Back", 'back')])

            action = menu.get_selection()
            if action == 'add':
                keyboard_from = OnScreenKeyboard("Enter FROM station:")
                from_station = keyboard_from.get_input()
                if from_station:
                    keyboard_to = OnScreenKeyboard("Enter TO station:")
                    to_station = keyboard_to.get_input()
                    if to_station:
                        preset['routes'].append((from_station, to_station))
                        self.show_message(f"Added route: {from_station} → {to_station}")

            elif isinstance(action, int):
                # Edit existing route
                route_menu = Menu(f"Edit Route {action+1}", [
                    (f"From: {preset['routes'][action][0]}", 'from'),
                    (f"To: {preset['routes'][action][1]}", 'to'),
                    ("Delete route", 'delete'),
                    ("Back", 'back')
                ])
                route_action = route_menu.get_selection()

                if route_action == 'from':
                    keyboard = OnScreenKeyboard("Enter FROM station:")
                    keyboard.text = preset['routes'][action][0]
                    new_from = keyboard.get_input()
                    preset['routes'][action] = (new_from, preset['routes'][action][1])
                    self.show_message("Route updated")

                elif route_action == 'to':
                    keyboard = OnScreenKeyboard("Enter TO station:")
                    keyboard.text = preset['routes'][action][1]
                    new_to = keyboard.get_input()
                    preset['routes'][action] = (preset['routes'][action][0], new_to)
                    self.show_message("Route updated")

                elif route_action == 'delete':
                    del preset['routes'][action]
                    self.show_message("Route deleted")

        elif preset_num == 4:
            self.show_message("Clock preset has no configuration options.")

    def show_message(self, message, duration=2):
        """Display a message to the user"""
        clear_screen()
        print(f"\n{Colors.BOLD}{Colors.CYAN}{'═' * 52}{Colors.RESET}")
        print(f"{Colors.BOLD}{Colors.CYAN}  Message{Colors.RESET}")
        print(f"{Colors.BOLD}{Colors.CYAN}{'═' * 52}{Colors.RESET}\n")
        print(f"  {Colors.GREEN}{message}{Colors.RESET}\n")
        print(f"{Colors.CYAN}{'─' * 52}{Colors.RESET}")
        time.sleep(duration)

    def configure_loop(self):
        """Configure auto-loop settings using 3-button interface"""
        while True:
            status = "ON" if self.loop_enabled else "OFF"
            menu = Menu("Configure Auto-Loop", [
                (f"Status: {status}", 'toggle'),
                (f"Interval: {self.loop_interval}s", 'interval'),
                (f"Presets: {','.join(map(str, self.loop_presets))}", 'presets'),
                ("Back", 'back')
            ])

            action = menu.get_selection()

            if action == 'toggle':
                toggle_menu = Menu("Enable Auto-Loop?", [
                    ("Yes", True),
                    ("No", False)
                ])
                self.loop_enabled = toggle_menu.get_selection()
                self.show_message(f"Auto-loop: {'ON' if self.loop_enabled else 'OFF'}")

            elif action == 'interval':
                picker = NumberPicker("Loop interval (seconds):", self.loop_interval, min_value=5, max_value=300)
                self.loop_interval = picker.get_value()
                self.show_message(f"Interval set to: {self.loop_interval}s")

            elif action == 'presets':
                # Multi-select menu for presets
                preset_menu = Menu("Select presets to loop", [
                    (f"Preset 1: {self.presets[1]['name']}", 1),
                    (f"Preset 2: {self.presets[2]['name']}", 2),
                    (f"Preset 3: {self.presets[3]['name']}", 3),
                    (f"Preset 4: {self.presets[4]['name']}", 4),
                    ("Done", 'done')
                ])

                new_presets = []
                while True:
                    result = preset_menu.get_selection()
                    if result == 'done':
                        break
                    elif isinstance(result, int):
                        if result in new_presets:
                            new_presets.remove(result)
                            self.show_message(f"Removed preset {result}", 1)
                        else:
                            new_presets.append(result)
                            self.show_message(f"Added preset {result}", 1)

                if new_presets:
                    self.loop_presets = sorted(new_presets)
                    self.loop_current_index = 0
                    if self.loop_enabled and self.loop_presets:
                        self.current_preset = self.loop_presets[0]
                    self.show_message(f"Presets: {','.join(map(str, self.loop_presets))}")

            elif action == 'back':
                break

    def refresh_data_background(self):
        """Refresh data in background for presets 2 and 3"""
        preset = self.presets[self.current_preset]
        current_time = time.time()

        if self.current_preset == 2:
            # Refresh multiple destinations
            for dest in preset['destinations']:
                if not self.running:
                    break
                data = get_connections(preset['from'], dest, limit=1)
                if data and data.get('connections'):
                    with self.cache_lock:
                        self.cache_destinations[dest] = data['connections'][0]
                        self.cache_destinations_timestamp[dest] = current_time

        elif self.current_preset == 3:
            # Refresh multiple routes
            for from_st, to_st in preset['routes']:
                if not self.running:
                    break
                data = get_connections(from_st, to_st, limit=1)
                if data and data.get('connections'):
                    with self.cache_lock:
                        self.cache_routes[(from_st, to_st)] = data['connections'][0]
                        self.cache_routes_timestamp[(from_st, to_st)] = current_time

        with self.cache_lock:
            self.preset_last_refresh[self.current_preset] = current_time
        self.is_refreshing = False

    def start_background_refresh(self):
        """Start background refresh if not already running"""
        if not self.is_refreshing and self.current_preset in [2, 3]:
            self.is_refreshing = True
            self.refresh_thread = threading.Thread(target=self.refresh_data_background, daemon=True)
            self.refresh_thread.start()

    def is_data_stale(self, preset_num):
        """Check if preset data needs refresh"""
        if self.update_interval == 0:
            return False  # Auto-refresh disabled

        current_time = time.time()
        last_refresh = self.preset_last_refresh.get(preset_num, 0)

        # Data is stale if it's older than update_interval
        return (current_time - last_refresh) >= self.update_interval

    def should_refresh_for_loop(self):
        """Calculate if we should refresh based on loop mode"""
        if not self.loop_enabled or self.update_interval == 0:
            return False

        # Calculate time to cycle through all loop presets
        total_loop_time = self.loop_interval * len(self.loop_presets)

        # If total loop time is less than update interval, stagger refreshes
        if total_loop_time < self.update_interval:
            # Refresh each preset when it becomes active and is stale
            return self.is_data_stale(self.current_preset)
        else:
            # Refresh more frequently to ensure all presets stay fresh
            return self.is_data_stale(self.current_preset)

    def refresh_data(self):
        """Fetch fresh data only if stale"""
        preset = self.presets[self.current_preset]
        current_time = time.time()

        # Check if refresh is needed
        if not self.is_data_stale(self.current_preset):
            return False

        if self.current_preset == 1:
            # Check if we have cached data
            if self.cache_preset1 and not self.is_data_stale(1):
                self.all_connections = self.cache_preset1
                return True

            data = get_connections(preset['from'], preset['to'], limit=10)
            if data and data.get('connections'):
                with self.cache_lock:
                    self.all_connections = data['connections']
                    self.cache_preset1 = data['connections']
                    self.cache_preset1_timestamp = current_time
                    self.preset_last_refresh[1] = current_time
                self.last_update = current_time
                return True

        elif self.current_preset in [2, 3]:
            # Start background refresh for presets 2 and 3
            self.start_background_refresh()
            self.last_update = current_time
            return True

        return False
    
    def display(self):
        """Main display function"""
        clear_screen()

        # Auto-loop logic
        current_time = time.time()
        if self.loop_enabled and not self.is_browsing and self.loop_presets:
            time_since_switch = current_time - self.loop_last_switch
            if time_since_switch >= self.loop_interval:
                # Switch to next preset in loop
                self.loop_current_index = (self.loop_current_index + 1) % len(self.loop_presets)
                self.current_preset = self.loop_presets[self.loop_current_index]
                self.loop_last_switch = current_time
                self.connection_index = 0
                # Don't clear all_connections - we'll use cache if available

        # No automatic refresh here - let each preset decide based on staleness

        # Header
        preset = self.presets[self.current_preset]
        print(f"{Colors.BOLD}{Colors.MAGENTA}{'═' * 52}{Colors.RESET}")
        loop_indicator = f" {Colors.YELLOW}[LOOP]{Colors.RESET}" if self.loop_enabled else ""
        print(f"{Colors.BOLD}{Colors.MAGENTA}  Preset {self.current_preset}/4: {preset['name']}{loop_indicator}{Colors.RESET}")
        print(f"{Colors.BOLD}{Colors.MAGENTA}{'═' * 52}{Colors.RESET}")

        # Display current preset
        if self.current_preset == 1:
            # Try to use cached data first
            if self.cache_preset1 and not self.is_data_stale(1):
                self.all_connections = self.cache_preset1
            elif not self.all_connections or self.is_data_stale(1):
                # Only refresh if no cache or data is stale
                self.refresh_data()

            if self.all_connections:
                if self.connection_index >= len(self.all_connections):
                    self.connection_index = 0

                display_connection(self.all_connections[self.connection_index], self.is_browsing)

                if self.is_browsing:
                    print(f"\n  {Colors.YELLOW}📖 Browsing: Connection {self.connection_index + 1}/{len(self.all_connections)}{Colors.RESET}")

        elif self.current_preset == 2:
            # Use cached data
            with self.cache_lock:
                cached = self.cache_destinations.copy()
            display_multiple_destinations(self.presets[2]['from'], self.presets[2]['destinations'], cached)
            # Start background refresh only if data is stale
            if not self.is_refreshing and self.is_data_stale(2):
                self.start_background_refresh()

        elif self.current_preset == 3:
            # Use cached data
            with self.cache_lock:
                cached = self.cache_routes.copy()
            display_multiple_routes(self.presets[3]['routes'], cached)
            # Start background refresh only if data is stale
            if not self.is_refreshing and self.is_data_stale(3):
                self.start_background_refresh()
        
        elif self.current_preset == 4:
            display_clock()
        
        # Footer controls
        print(f"\n{Colors.CYAN}{'─' * 52}{Colors.RESET}")

        if self.auto_mode:
            status_parts = []

            # Show loop status
            if self.loop_enabled:
                time_until_switch = max(0, int(self.loop_interval - (current_time - self.loop_last_switch)))
                status_parts.append(f"Loop: {time_until_switch}s")

            # Show refresh status based on current preset
            if self.update_interval > 0 and not self.is_browsing:
                preset_last = self.preset_last_refresh.get(self.current_preset, 0)
                time_since_refresh = current_time - preset_last
                next_update = max(0, int(self.update_interval - time_since_refresh))
                status_parts.append(f"Refresh: {next_update}s")
            elif self.update_interval == 0:
                status_parts.append(f"Refresh: OFF")

            if status_parts:
                print(f"{' | '.join(status_parts)}")

            print(f"{Colors.YELLOW}[o] Menu  [U] Prev  [D] Next  [O] Config{Colors.RESET}")
        else:
            print(f"{Colors.YELLOW}[u] Up  [d] Down  [o] Select{Colors.RESET}")
    
    def show_menu(self):
        """Show main menu using 3-button interface"""
        refresh_status = f"{self.update_interval}s" if self.update_interval > 0 else "OFF"
        menu = Menu("Main Menu", [
            ("Previous Preset", 'prev_preset'),
            ("Next Preset", 'next_preset'),
            ("Select Preset", 'select_preset'),
            ("Browse Connections (Preset 1)", 'browse'),
            ("Configure Current Preset", 'config_preset'),
            ("Configure Auto-Loop", 'config_loop'),
            (f"Auto-Refresh Interval: {refresh_status}", 'config_refresh'),
            ("Force Refresh Now", 'refresh'),
            ("Back to Display", 'back'),
            ("Quit", 'quit')
        ])

        return menu.get_selection()
    
    def handle_command(self, cmd):
        """Handle menu command"""
        if cmd == 'prev_preset':
            self.current_preset = max(1, self.current_preset - 1)
            self.is_browsing = False
            self.connection_index = 0
            self.all_connections = []

        elif cmd == 'next_preset':
            self.current_preset = min(4, self.current_preset + 1)
            self.is_browsing = False
            self.connection_index = 0
            self.all_connections = []

        elif cmd == 'select_preset':
            preset_menu = Menu("Select Preset", [
                (f"1. {self.presets[1]['name']}", 1),
                (f"2. {self.presets[2]['name']}", 2),
                (f"3. {self.presets[3]['name']}", 3),
                (f"4. {self.presets[4]['name']}", 4),
                ("Back", 'back')
            ])
            preset = preset_menu.get_selection()
            if isinstance(preset, int):
                self.current_preset = preset
                self.is_browsing = False
                self.connection_index = 0
                self.all_connections = []

        elif cmd == 'browse':
            if self.current_preset == 1:
                if not self.all_connections:
                    self.refresh_data()
                if self.all_connections:
                    self.is_browsing = True
                    browse_menu = Menu(f"Browse Connections ({len(self.all_connections)} found)", [
                        (f"{i+1}. {format_time(conn['from']['departure'])}", i)
                        for i, conn in enumerate(self.all_connections)
                    ] + [("Back", 'back')])

                    result = browse_menu.get_selection()
                    if isinstance(result, int):
                        self.connection_index = result
                    self.is_browsing = True
            else:
                self.show_message("Browsing only available for Preset 1", 1)

        elif cmd == 'config_preset':
            self.configure_preset(self.current_preset)
            self.connection_index = 0
            self.all_connections = []

        elif cmd == 'config_loop':
            self.configure_loop()
            self.loop_last_switch = time.time()

        elif cmd == 'config_refresh':
            self.configure_auto_refresh()

        elif cmd == 'refresh':
            self.is_browsing = False
            self.connection_index = 0
            self.all_connections = []
            # Clear all caches and timestamps
            with self.cache_lock:
                self.cache_preset1 = []
                self.cache_preset1_timestamp = 0
                self.cache_destinations.clear()
                self.cache_destinations_timestamp.clear()
                self.cache_routes.clear()
                self.cache_routes_timestamp.clear()
                self.preset_last_refresh = {1: 0, 2: 0, 3: 0, 4: 0}
            self.refresh_data()
            self.show_message("All data refreshed!", 1)

        elif cmd == 'quit':
            self.running = False

        elif cmd == 'back':
            pass  # Just return to display

    def configure_auto_refresh(self):
        """Configure auto-refresh interval with warning for short intervals"""
        current_value = self.update_interval if self.update_interval > 0 else 60

        picker = NumberPicker(
            "Auto-Refresh Interval (0=OFF):",
            current_value,
            min_value=0,
            max_value=300
        )
        new_interval = picker.get_value()

        # Show warning for intervals < 30s
        if 0 < new_interval < 30:
            clear_screen()
            print(f"\n{Colors.BOLD}{Colors.YELLOW}{'═' * 52}{Colors.RESET}")
            print(f"{Colors.BOLD}{Colors.YELLOW}  Warning!{Colors.RESET}")
            print(f"{Colors.BOLD}{Colors.YELLOW}{'═' * 52}{Colors.RESET}\n")
            print(f"{Colors.YELLOW}  A refresh interval less than 30 seconds can:{Colors.RESET}")
            print(f"{Colors.YELLOW}  • Cause excessive API requests{Colors.RESET}")
            print(f"{Colors.YELLOW}  • Lead to rate limiting or IP blocking{Colors.RESET}")
            print(f"{Colors.YELLOW}  • Drain battery on portable devices{Colors.RESET}\n")
            print(f"{Colors.CYAN}{'─' * 52}{Colors.RESET}")

            confirm_menu = Menu("Continue with this interval?", [
                (f"Yes, set to {new_interval}s", True),
                ("No, choose again", False),
                ("Cancel", 'cancel')
            ])
            result = confirm_menu.get_selection()

            if result == False:
                # Recursive call to choose again
                self.configure_auto_refresh()
                return
            elif result == 'cancel':
                return

        self.update_interval = new_interval
        status = f"{new_interval}s" if new_interval > 0 else "OFF"
        self.show_message(f"Auto-refresh set to: {status}")

def main():
    display = TrainDisplay()

    clear_screen()
    print(f"{Colors.BOLD}{Colors.GREEN}Swiss Train Display - 3 Button Interface{Colors.RESET}")
    print(f"{Colors.YELLOW}Controls: u=up, d=down, o=ok{Colors.RESET}")
    print(f"{Colors.YELLOW}Long press: U=prev preset, D=next preset, O=config{Colors.RESET}\n")
    print(f"{Colors.YELLOW}Loading initial data...{Colors.RESET}\n")
    time.sleep(2)

    display.refresh_data()

    try:
        while display.running:
            if display.auto_mode:
                # Auto mode: display and wait for button press
                display.display()

                # Wait for input
                if sys.platform != 'win32':
                    # Unix-like systems: check for input without blocking
                    ready = select.select([sys.stdin], [], [], 1)
                    if ready[0]:
                        key = sys.stdin.readline().strip()

                        # Handle long-press shortcuts in auto mode
                        if key == 'U':  # Long up = Previous preset
                            display.current_preset = max(1, display.current_preset - 1)
                            display.connection_index = 0
                            display.all_connections = []
                            display.is_browsing = False
                        elif key == 'D':  # Long down = Next preset
                            display.current_preset = min(4, display.current_preset + 1)
                            display.connection_index = 0
                            display.all_connections = []
                            display.is_browsing = False
                        elif key == 'O':  # Long OK = Quick config
                            display.configure_preset(display.current_preset)
                            display.connection_index = 0
                            display.all_connections = []
                        elif key == 'o':  # Short OK = Menu
                            display.auto_mode = False
                else:
                    time.sleep(1)
            else:
                # Menu mode
                cmd = display.show_menu()
                display.handle_command(cmd)

                # Return to auto mode if going back
                if cmd == 'back':
                    display.auto_mode = True

    except KeyboardInterrupt:
        pass

    clear_screen()
    print(f"\n{Colors.GREEN}Thanks for using Swiss Train Display!{Colors.RESET}\n")

if __name__ == "__main__":
    main()
