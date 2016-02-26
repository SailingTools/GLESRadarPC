#! /usr/bin/python

"""
Class for handling interaction with a Koden RADARpc

Radar does 10 revolutions in 22.6s (~2.25s per revolution)

// Gains [0:10:90,99,'auto']
// Rsponse bytes [20] (auto off=0 or on=01) and byte [21] (value)
GAINS = [
24:47:00:0d
24:47:0a:0d
24:47:14:0d
24:47:1e:0d
24:47:28:0d
24:47:32:0d
24:47:3c:0d
24:47:46:0d
24:47:50:0d
24:47:5a:0d
24:47:63:0d
24:67:11:0d

// Seas [0:10:90,99,'auto']
// Rsponse bytes [26] (auto off=00 or on=02) and byte [27] (value)
24:53:00:0d
24:53:0a:0d
24:53:14:0d
24:53:1e:0d
24:53:28:0d
24:53:32:0d
24:53:3c:0d
24:53:46:0d
24:53:50:0d
24:53:5a:0d
24:53:63:0d
24:80:22:0d

// Rain [0:10:90,99]
// Rsponse bytes [24] (value)
26:46:00:0d
26:46:0a:0d
26:46:14:0d
26:46:1e:0d
26:46:28:0d
26:46:32:0d
26:46:3c:0d
26:46:46:0d
26:46:50:0d
26:46:5a:0d
26:46:63:0d

// Short/Long Pulse
// Response bytes [18] (long=01, short=00)
Long_Pulse  = 26:a4:01:0d
Short_Pulse = 26:a4:00:0d 

"""

UDP_IP = "192.168.0.1"
UDP_PORT = 10001
MESSAGE_01 = '\x26\xa7\x11\x0d'        # Sent every 5 seconds in standby
MESSAGE_02 = '\x26\x74\x11\x0d'  	   # Sent when turning on scanning
MESSAGE_03 = '\x26\xab\x11\x0d'        # Sent every 5 seconds while scanning
MESSAGE_04 = '\x26\xaa\x00\x00\x0d'    # Sent before turning off scanning (?)
MESSAGE_05 = '\x26\x74\x00\x0d'        # Sent when turning off scanning
MESSAGE_06 = '\x24\x80\x22\x0d'        # Auto-sea turned on
MESSAGE_07 = '\x24\x67\x11\x0d'        # Auto-gain turned on


RANGES = [
'\x26\x20\x00\xe7\x0d',     # 1/8 - 231m
'\x26\x20\x01\xcf\x0d',     # 1/4 - 463m
'\x26\x20\x03\x9e\x0d',     # 1/2 - 926m
'\x26\x20\x05\x6d\x0d',     # 3/4 - 1389m
'\x26\x20\x07\x3c\x0d',     # 1.0 - 1852m
'\x26\x20\x0a\xda\x0d',     # 1.5 - 2778m
'\x26\x20\x0e\x78\x0d',     # 2.0 - 2704m
'\x26\x20\x15\xb4\x0d',     # 3.0 - 5556m
'\x26\x20\x1c\xf0\x0d',     # 4.0 - 7408m
'\x26\x20\x2b\x68\x0d',     # 6.0 - 11112m
'\x26\x20\x39\xe0\x0d',     # 8.0 - 14816m
'\x26\x20\x56\xd0\x0d',     # 12 - 22224m
'\x26\x20\x73\xc0\x0d',     # 16 - 29632m
'\x26\x20\xad\xa0\x0d',     # 24 - 44448m
'\x26\x20\xe7\x80\x0d',     # 32 - 59264m
'\x26\x21\x04\x70\x0d',]    # 36 - 66672m

"""
# Equation to get range in meters:
for R in RANGES:
    dist_meters = int(R[1:4].encode('hex'),16)-2097152
    dist_nm = dist_meters/1852.0
    print("%i m, %f nm"%(dist_meters, dist_nm))

Range index number is returned in the bytes [16] & [17] of the string from the radar
"""

# STARTUP:
MESSAGES = [
'\x26\xfe\xff\xff\xff\x00\x0d',
'\x26\xff\x11\x0d',
'\x24\x4e\x11\x0d',
'\x26\xfe\xff\xff\xff\x00\x0d',
'\x26\xff\x11\x0d',
'\x26\x72\xff\x0d',
'\x26\xff\x11\x0d',
'\x26\x9a\x11\x0d',
'\x26\xad\x02\x0d',
'\x26\xac\x00\x0d',
'\x26\xa5\x00\x0d',
'\x26\x99\x11\x0d',
'\x24\x41\x22\x0d',
'\x26\x88\x04\x0d',
'\x26\x85\x55\x0d',
'\x24\x67\x00\x0d',
'\x26\x82\x08\x0d',
'\x26\x81\x07\x0d',
'\x24\x80\x00\x0d',
'\x26\x84\x0a\x0d',
'\x26\x83\x0a\x0d',
'\x26\x87\x07\x0d',
'\x26\x30\x03\xd3\x0d',
'\x26\x9c\x00\x00\x00\x00\x0d',
'\x26\x44\xa5\x0d',
'\x26\x20\x01\xcf\x0d',
'\x24\x47\x32\x0d',
'\x24\x53\x14\x0d',
'\x26\x46\x32\x0d',
'\x26\x49\x33\x0d',
'\x26\x45\x00\x0d',
'\x26\xa4\x00\x0d',
'\x24\x47\x32\x0d',
'\x24\x53\x14\x0d',
'\x26\x46\x32\x0d',
'\x26\x49\x33\x0d',
]

import socket
import sys
import time
import threading
import Queue
import pickle
from math import pi

import matplotlib
import matplotlib.pyplot as plt

def byteswap(hexstring):
    return hexstring[::-1]

def intarray(hexstring):
    return [int(b.encode('hex'),16) for b in hexstring]

class KodenRadarPC():

    def __init__(self):
        self.timeout = 0.1
        self.last_work_time = time.time()
        self.socket = None

        self.recv_thread = threading.Thread(target=self.recieve)
        self.recv_thread.daemon = True
        self.recv_thread.start()

        self.status = 0
        self.UDP_IP = "192.168.0.1"
        self.UDP_PORT = 10001
        
        self.recordOutput = False
        self.output = []
        self.curr_scan = []
        self.prev_scan = []
        self.prev_angle = 0

        self.terminate = False
            
    def start_interactive(self, menu=True):
        if menu:
            self.input_thread = threading.Thread(target=self.input_loop)
            self.input_thread.daemon = True
            self.input_thread.start()

        self.main_loop()

    def run_for_time(self, t=30):
        self.run_command('c')
        time.sleep(10)
        self.run_command('s')
        time.sleep(t)
        self.run_command('k')
        time.sleep(10)
        self.run_command('d')
        return        

    def connect(self):
        print('Creating Connection to RADAR')
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # Internet, UDP
        #s.setblocking(0)
        s.settimeout(10)
        s.connect((self.UDP_IP, self.UDP_PORT))
        self.socket = s
        print('Connected to RADAR')
        return

    def close(self):
        self.socket.close()
        self.socket = None
    
    def init_plot(self):
        #plt.ion()
        print("Initiating figure window")
        self.updatePlot = False
        self.fig, self.ax = plt.subplots(subplot_kw=dict(projection='polar'))
        self.ax.set_theta_zero_location("N")
        self.ax.set_theta_direction(-1)
        plt.draw()
        plt.pause(0.01)
        return

    def plot_radar(self):
        theta = [2.0*pi*self.get_angle(o)/2050.0 for o in self.prev_scan]
        R = range(256)
        values = []
        for o in self.prev_scan:
            values.append(intarray(o[-256:]))
        # Add in the first value (to avoid a gap)
        theta.append(2*pi)
        values.append(values[0])
        # Transpose the values for plotting
        fvalues = zip(*values)
        cax = self.ax.contourf(theta, R, fvalues, 5)
        self.ax.set_ylim(0, 256)
        #cb = fig.colorbar(cax)
        #cb.set_label("Pixel reflectance")
        #self.fig.show()
        print("Drawing Radar figure window")
        plt.draw()
        plt.pause(0.01)
        return

    def idle_work(self):
        # Ping every 5 seconds
        now = time.time()
        if now - self.last_work_time > 5:
            #print('Current status: %i'%(self.status))
            if self.socket:
                if self.status:
                    self.ping_running()
                else:
                    self.ping_standby()
            self.last_work_time = now
        return

    def start_radar(self):
        print('Starting RADAR Scanning')
        self.socket.sendall('\x26\x74\x11\x0d')
        self.status = 1

    def stop_radar(self):
        print('Stopping RADAR Scanning')       
        self.socket.sendall('\x26\x74\x00\x0d')
        self.status = 0

    def ping_standby(self):
        print('Pinging RADAR in Standby')
        self.socket.sendall('\x26\xa7\x11\x0d')

    def ping_running(self):
        print('Pinging RADAR while Running')
        self.socket.sendall('\x26\xab\x11\x0d')

    def initiate(self):
        print('Adding initiation messages to queue.')
        STARTUP_MESSAGES = [
            '\x26\xfe\xff\xff\xff\x00\x0d',
            #'\x26\xff\x11\x0d',
            '\x24\x4e\x11\x0d',
            '\x26\xfe\xff\xff\xff\x00\x0d',
            #'\x26\xff\x11\x0d',
            '\x26\x72\xff\x0d',
            #'\x26\xff\x11\x0d',
            '\x26\x9a\x11\x0d',
            '\x26\xad\x02\x0d',
            '\x26\xac\x00\x0d',
            '\x26\xa5\x00\x0d',
            '\x26\x99\x11\x0d',
            '\x24\x41\x22\x0d',
            '\x26\x88\x04\x0d',
            '\x26\x85\x55\x0d',
            '\x24\x67\x00\x0d',     # 24:67:11:0d
            '\x26\x82\x08\x0d',
            '\x26\x81\x07\x0d',
            '\x24\x80\x00\x0d',     # 24:80:11:0d
            '\x26\x84\x0a\x0d',
            '\x26\x83\x0a\x0d',
            '\x26\x87\x07\x0d',
            '\x26\x30\x03\xd3\x0d',
            '\x26\x9c\x00\x00\x00\x00\x0d',
            '\x26\x44\xa5\x0d',
            '\x26\x20\x01\xcf\x0d', # 26:20:0a:da:0d
            '\x24\x47\x32\x0d',
            '\x24\x53\x14\x0d',
            '\x26\x46\x32\x0d',
            '\x26\x49\x33\x0d',
            '\x26\x45\x00\x0d',
            '\x26\xa4\x00\x0d',
            '\x24\x47\x32\x0d',     #
            '\x24\x53\x14\x0d',     #
            '\x26\x46\x32\x0d',     #
            '\x26\x49\x33\x0d',     #
            ]
        for m in STARTUP_MESSAGES:
            self.socket.sendall(m)
            time.sleep(0.1)

    def send_message(self, m):
        print('Sending messgage: %s'%(m.encode('hex')))
        self.socket.sendall(m)

    def scan_output(self):
        for d in self.output:
            self.save_scan(d, draw=False)

    def get_angle(self, d):
        return int(byteswap(d[42:44]).encode('hex'),16)

    def save_scan(self, d, draw=True):
        if not len(d) > 1000:
            return
        angle = int(byteswap(d[42:44]).encode('hex'),16)
        #print("Processing angle: %i"%(angle))
        if angle < self.prev_angle:
            print("Saving Scan Image (time=%f)"%(time.time()))
            self.prev_scan = self.curr_scan
            self.curr_scan = []
            if draw:
                self.updatePlot = True
        self.curr_scan.append( d )
        self.prev_angle = angle

    def recieve(self):
        print("Listening for input")
        counter = 0
        while True:
            if not self.socket:
                continue

            try:
                d = self.socket.recv(3000)
                if len(d) < 1000:
                    print('[%i] Received %i bytes'%(counter, len(d)))
                if self.recordOutput:
                    self.output.append(d)
                self.save_scan(d)
                counter += 1
            except:
                print('Error recieving data.  Stopped receiving.')
                return

    def save_output(self, fname='output'):
        print('Writing output to %s (%i lines)'%(fname, len(self.output)))
        if self.recordOutput:
            f = open(fname, 'wb')
            pickle.dump(self.output, f)
            f.close()

    def load_output(self, fname='output'):
        print('Loading output from %s'%(fname))
        self.output = []
        f = open(fname, 'rb')
        self.output = pickle.load(f)
        f.close()

    def toggle_record(self):
        if self.recordOutput:
            self.recordOutput = False
            print('Turning Record OFF')
        else:
            self.recordOutput = True
            print('Turning Record ON') 

    def update_plot(self):
        self.updatePlot = True

    def run_command(self, i):
        options = {'c'  : self.connect,
                   'd'  : self.close, 
                   's'  : self.start_radar,
                   'i'  : self.initiate,
                   'k'  : self.stop_radar,
                   'p'  : self.ping_standby,
                   'r'  : self.ping_running,
                   'm'  : self.send_message,
                   'o'  : self.toggle_record,
                   'os' : self.save_output,
                   'ol' : self.load_output,
                   'oc' : self.scan_output,
                   'pl' : self.update_plot,
            }
        if ':' in i:
            (f,a) = i.split(':')
            options[f](a)
        else:
            options[i]()
        return

    def input_loop(self):
        while True:
            i = raw_input('Enter a command ("h" for help):')
            time.sleep(1)
            if i == "h":
                self.print_help()
            elif i == "q":
                self.terminate = True
            else:
                try:
                    self.run_command(i)
                except:
                    print("Command not found: %s"%(i))
        print('Exiting working loop.')

    def main_loop(self):
        self.init_plot()

        while not self.terminate:
            # Update the figure window if requested
            if self.updatePlot:
                self.plot_radar()
                self.updatePlot = False
            # Ping every 5 seconds
            self.idle_work()

        if self.socket:
            self.close()
        print("Exiting the program")
        plt.close()

    def print_help(self):
        print("""
c   =   Open connection to Radar
d   =   Close connection to Radar
i   =   Send initiation messages to Radar
s   =   Start Radar scanning
k   =   Stop Radar scanning
p   =   Ping Radar in Standby
r   =   Ping Radar while Running
m:a =   Send arbitrary message "a" (string)
o   =   Toggle Record Output
os  =   Save output to file
ol  =   Load output from file
oc  =   Scan the current output data
pl  =   Redraw the current radar plot
q   =   Quit
""")

if __name__ == "__main__":
    r = KodenRadarPC()
    r.start_interactive()


