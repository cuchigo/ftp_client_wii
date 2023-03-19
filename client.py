import socket
import threading
import time
import os
import sys

os.chdir("C:/Users/Edgar/Documents/my_app_wii/")


HOST = "192.168.1.184" 
PORT = 21  
commands = ['USER usu','PASS contra','TYPE I','PWD','PASV','CWD Users/Edgar/Documents']

def download(file):
    global s
    s.sendall('RETR {}\r\n'.format(file).encode())
        
    while True:
        answer = s.recv(1024).decode().replace('\r\n','')
        print(answer)
        if '226' in answer:
            break
    return

def is_port_in_use(port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        return s.connect_ex(('localhost', port)) == 0

def PD():
    global port_pd
    try:
        
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST, int(port_pd)))
            while True:
                data = s.recv(1000)
                
                with open("Default.rdp",'ab+') as a:
                    a.write(data)
                
                
    except:
        
        while True:
            if is_port_in_use(port_pd):
                PD()
                break
            else:
                time.sleep(3)
        



mi_hilo = threading.Thread(target=PD)
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    if s.recv(1024) != "":
        print("Conectado!")
        for c in commands: #Comandos de configuración
            s.send(c.encode() + b"\r\n")
            if c == 'LIST':
                answer = s.recv(30000).decode().replace('\r\n','')
                
                
                print(answer)
            answer = s.recv(1024).decode().replace('\r\n','')
            if int(answer[:3]) == 227:
                port_ip = answer.replace('.','').replace(')','').split(',')
                port_pd = (int(port_ip[4]) * 256) + int(port_ip[5])
                print(f"Puerto pasivo: {port_pd}")
                mi_hilo.start()
                
            print(answer)

        download("Default.rdp")
        s.send(b'NOOP\r\n')
        print(s.recv(1024).decode().replace('\r\n',''))
        s.send(b'QUIT\r\n')
        print(s.recv(1024).decode().replace('\r\n',''))
        
        
    s.close()
sys.exit()
