#! /usr/bin/python

"""
Class for handling interaction with a Koden RADARpc

Radar does 10 revolutions in 22.6s (~2.25s per revolution)
"""

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


