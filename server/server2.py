import socket               # Import socket module
import thread
import pyrebase
import time
import datetime


def on_new_client(clientsocket, addr, db):
    match = "ieee"
    while True:
        msg = clientsocket.recv(1024)
        clientsocket.send(msg)
        print addr, ' >> ', msg
        if len(msg) < 4:
            break
        if msg[0] == "R":
            if msg[3] == "1":
                ts = time.time()
                st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
                match = st  
        thread.start_new_thread(upload_task, (msg,match,))

    clientsocket.close()

def clean_task():
   db.child("match2").set("") 

def upload_task(msg,match):
    
    if msg[0] == "C":
        print "Coordinates"
        db.child("matches").child(match).child("coordinates").set(msg[3:])
        db.child("match2").child("coordinates").set(msg[3:])
        if msg[8:] == "000":
            db.child("match2").child("coordinatesCode").set("1")
        if msg[8:] == "001":
            db.child("match2").child("coordinatesCode").set("2")
        if msg[8:] == "010":
            db.child("match2").child("coordinatesCode").set("3")
        if msg[8:] == "011":
            db.child("match2").child("coordinatesCode").set("4")
        if msg[8:] == "100":
            db.child("match2").child("coordinatesCode").set("5")
        if msg[8:] == "101":
            db.child("match2").child("coordinatesCode").set("6")
        if msg[8:] == "110":
            db.child("match2").child("coordinatesCode").set("7")
        if msg[8:] == "111":
            db.child("match2").child("coordinatesCode").set("8")
    if msg[0] == "T":
        print "Time"
        db.child("matches").child(match).child("time").set(msg[3:])
        db.child("match2").child("time").set(msg[3:])
    if msg[0] == "S":
        print "Stage"
        if msg[3] == "1":
            db.child("matches").child(match).child("stagesCompleted").child("A").set("complete")
            db.child("match2").child("stagesCompleted").child("A").set("complete")
        if msg[3] == "2":
            db.child("matches").child(match).child("stagesCompleted").child("B").set("complete")
            db.child("match2").child("stagesCompleted").child("B").set("complete")

        if msg[3] == "3":
            db.child("matches").child(match).child("stagesCompleted").child("C").set("complete")
            db.child("match2").child("stagesCompleted").child("C").set("complete")
    if msg[0] == "E":
        print "Encoder"
        if msg[3] == "1":
            db.child("matches").child(match).child("encoder").set("red")
            db.child("match2").child("encoder").set("red")
        if msg[3] == "2":
            db.child("matches").child(match).child("encoder").set("Green")
            db.child("match2").child("encoder").set("Green")
        if msg[3] == "3":
            db.child("matches").child(match).child("encoder").set("Yellow")
            db.child("match2").child("encoder").set("Yellow")

    if msg[0] == "B":
        print "Gangplank"
        db.child("matches").child(match).child("gangplank").set(msg[3:])
    if msg[0] == "R":
        if msg[3] == "1":
            db.child("matches").child(match).child("board").set(addr)
        else:
            db.child("matches").child(match).child("Reset").set(msg[3:])
            #db.child("match2").set("0") 
            
        print "Reset"
        
    if msg[0] == "P":
        print "Points"
        db.child("matches").child(match).child("points").set(msg[3:])

config = {
    "apiKey": "AIzaSyD6AO29mVQiqfhMWfYBwclAMbxpFtgwyJ4",
    "authDomain": "ieee-se-c.firebaseapp.com",
    "databaseURL": "https://ieee-se-c.firebaseio.com",
    "storageBucket": "ieee-se-c.appspot.com",
    "serviceAccount": "./ieee-se-c-firebase-adminsdk-yurbu-d78132e874.json"
}

firebase = pyrebase.initialize_app(config)
db = firebase.database()

s = socket.socket()         # Create a socket object
host = ''  # Get local machine name
port = 1334                # Reserve a port for your service.

print 'Server started!'
print 'Waiting for clients...'

s.bind((host, port))        # Bind to the port
s.listen(1)                 # Now wait for client connection.

while True:
    c, addr = s.accept()     # Establish connection with client.
    print 'Got connection from', addr
    thread.start_new_thread(on_new_client, (c, addr, db) )
    # Note it's (addr,) not (addr) because second parameter is a tuple
    # Edit: (c,addr)
    # that's how you pass arguments to functions when creating new threads using thread module.
s.close()
