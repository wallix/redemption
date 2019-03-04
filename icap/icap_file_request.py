#!/usr/bin/python
# -*- coding: utf-8 -*-
# kate: space-indent on; tab-width 4; indent-width 4; replace-tabs on; eol unix;

import socket
import argparse
import os




class ICAPFileRequest(object):

    def __init__(self, host, localhost):
        self.host      = host
        self.localhost = localhost
        self.port      = 1344

        self.file_size = 0
        self.current_data_size = 0
        self.icap_socket       = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.open = False



    def start(self, filename, file_size):
        self.file_size = file_size
        self.current_data_size = 0

        try:
            self.icap_socket.connect((self.host, self.port))
            #print "Connection on {}".format(self.host+":"+str(self.port))
            #print ""
     
        except:
            #print "Error can't connect on {}".format(self.host+":"+str(self.port))
            self.open = False

        request = self.write_request(filename, self.localhost, self.host, self.port)
        self.icap_socket.send(request.encode("utf-8"))
        #print request+"\r\n"
        self.open = True



    def write_request(self, filename, localhost, host, port):
        httpRequest   = "GET http://"+localhost+"/"+filename+" HTTP/1.1\r\n"
        httpRequest  += "Host: "+localhost+"\r\n" 
        httpRequest  += "\r\n"

        httpResponse  = "HTTP/1.1 200 OK\r\n"
        httpResponse += "Transfer-Encoding: chunked\r\n"
        #httpResponse += "Content-Length: "+str(self.file_size)+"\r\n" 
        httpResponse += "\r\n"

        icapRequest   = "RESPMOD icap://"+host+":"+str(port)+"/avscan ICAP/1.0\r\n"
        icapRequest  += "Allow: 204\r\n"   
        #icapRequest  += "Content-Length: "+str(self.file_size)+"\r\n" 
         #icapRequest  += "Preview: "+str(1024)+"\r\n"
        icapRequest  += "Connection: close\r\n"
        icapRequest  += "Host: "+host+"\r\n"
        icapRequest  += "Encapsulated: req-hdr=0, res-hdr="+str(len(httpRequest))+", res-body="+str(len(httpRequest) + (len(httpResponse)))+"\r\n" 
        icapRequest  += "\r\n"

        request = icapRequest + httpRequest + httpResponse

        return request



    def wait_response(self):

        msg_recu = b""
        response = b""
        step = 0
        #print "Waiting for response..."
        while msg_recu != b"0\r\n\r\n" and step < 50:
            msg_recu = self.icap_socket.recv(1024)
            step += 1
            if msg_recu != b"":  
                if msg_recu == "ICAP/1.0 100 Continue\r\n\r\n":
                    pass
                    #print msg_recu
                    #size_to_read = send_chunk(f, socket, size_to_read)
                else:
                    response += msg_recu

        res = "WAIT"
        if response:
            #print response
            response_lines = response.split("\r\n")
            
            if response_lines[0] == "ICAP/1.0 204 Unmodified":
               res = "ACCEPTED"
            else:
                if response_lines[0] == "ICAP/1.0 408 Request timeout":
                    res = "ERROR"
                else:
                    res = "REJECTED"
        else: 
            pass
            #print "Error step = " + str(step)
        #print "Close connection to {}".format(self.host+":"+str(self.port))
        self.icap_socket.close()
        
        return res
             


    def receive_data(self, data, size):

        chunked_content   = b""
        data_size_to_send = size
        current_offset    = 0

        chuncked_data = data[current_offset : size]
        chunked_content += str(hex(data_size_to_send).split('x')[-1])+"\r\n" +  chuncked_data + "\r\n"
        
        self.current_data_size += size
        is_eol = (self.file_size == self.current_data_size)

        if is_eol:
            chunked_content += "\r\n\r\n0\r\n\r\n"
            #print chunked_content
        
        self.icap_socket.send(chunked_content)

        res = "WAIT"

        if is_eol:
            self.file_size         = 0
            self.current_data_size = 0
            res = self.wait_response()

        return res



    
   

if __name__ == '__main__':

    host      = "127.0.0.1"
    localhost = "127.0.1.1"
            
    
    file_path = "/home/cmorolodo/Bureau/redemption/icap/README.md"
    file_name = "README.md"
    file_size = os.path.getsize(file_path)

    file_to_valid2 = ICAPFileRequest(host, localhost)
    file_to_valid2.start(file_name, file_size)

    res = ""
    f = open(file_path, "rb")

    while file_size > 0:
        
        file_size -= 2560 
        content = f.read(2560)
        res = file_to_valid2.receive_data(content, len(content))

    if res == "ACCEPTED":
        print "File is ACCEPTED, path: \"" + file_path + "\""
    else:
        if res == "REJECTED":
            print "File is REJECTED, path: \"" + file_path + "\""
        else:
            print "File analysis ERROR res=\""+res+"\", path \"" + file_path + "\""
 

    print ""
    print ""
    print ""
    
    file_path = "/home/cmorolodo/Bureau/redemption/icap/the_zeus_binary_chapros"
    file_name = "the_zeus_binary_chapros"
    file_size = os.path.getsize(file_path)

    file_to_valid1 = ICAPFileRequest(host, localhost)
    file_to_valid1.start(file_name, file_size)

    res = ""
    f = open(file_path, "rb")

    while file_size > 0:
        
        file_size -= 2560 
        content = f.read(2560)
        res = file_to_valid1.receive_data(content, len(content))

    if res == "ACCEPTED":
        print "File is ACCEPTED, path: \"" + file_path + "\""
    else:
        if res == "REJECTED":
            print "File is REJECTED, path: \"" + file_path + "\""
        else:
            print "File analysis ERROR res=\""+res+"\", path \"" + file_path + "\""

    

    

