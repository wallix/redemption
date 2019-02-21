#!/usr/bin/perl
use IO::Socket;
use Sys::Hostname; 
use File::Basename;
use Time::Local;
use URI::Escape;

use strict;
use warnings;

    if($#ARGV != 0) {
        usage();
        exit 1;
    }

    my $startTime = time();
    my $fileName = $ARGV[0];
    logmessage("Starting: $0 $fileName\n");
    logmessage("Filename:". basename($fileName)."\n");
    my $fileSize;
    unless(-e $fileName && ($fileSize = -s $fileName)) {
        logmessage("fileName does not exist: $fileName\n");
        exit 2;
    }    
    logmessage("Found: $fileName ($fileSize bytes)\n");

    my $scanResult="";
    $scanResult = VirusScanner();
    logmessage("scanResult: File is $scanResult\n");



sub usage {
    print "usage: $0 filename\n";
}

sub logmessage {
  my $logmessage = shift;
  open my $logfile, ">>", "icapClient.log" or die "Could not open icapClient.log: $!\n";
  print $logmessage;
  print $logfile ("[".localtime()."] ($$) ".$logmessage);
  close($logfile);
}

sub VirusScanner {

    my $server = "127.0.0.1";
    my $port = "1344";
    my $service = "avscan";
    my $version = "ICAP/1.0";

    my $sock = new IO::Socket::INET ( 
        PeerAddr => $server, 
        PeerPort => $port, 
        Proto => 'tcp', 
    ); die logmessage("ICAP server error:".$server.":".$port) unless $sock; 
    $sock->autoflush();

    #this should actually be the IP address of the host actually uploading the file,
    #but when this is run as client, it will use it's own IP address
    #if this were a cgi script, it would most likely be $ENV{'REMOTE_HOST'} 
    my ($clientIP) = inet_ntoa((gethostbyname(hostname()))[4]);

    my $httpRequest  = "GET http://" . $clientIP . "/" . time() . "/" . uri_escape(basename($fileName)) . " HTTP/1.1\r\n"; 
       $httpRequest .= "Host: " . $clientIP . "\r\n\r\n";    
    
    my $httpResponse = "HTTP/1.1 200 OK\r\n";
       $httpResponse .= "Transfer-Encoding: chunked\r\n";
       $httpResponse .= "Content-Length: $fileSize\r\n"; # don't really need the size because it's chunked
       $httpResponse .= "\r\n";
    
    my $icapRequest = "RESPMOD icap://" . $server . ":" . $port . "/" . $service . " " . $version . "\r\n";
       $icapRequest .= "Allow: 204\r\n";
       $icapRequest .= "Connection: close\r\n";
       $icapRequest .= "Host: " . $server . "\r\n";

       $icapRequest .= "Encapsulated: req-hdr=0, res-hdr=" . length($httpRequest) . ", res-body=" . (length($httpRequest) + length($httpResponse)) . "\r\n";
       $icapRequest .= "\r\n";
 
    #send ICAP & HTTP headers
    logmessage("Sending Headers>>\n");
    logmessage($icapRequest . $httpRequest . $httpResponse);
    $sock->send($icapRequest . $httpRequest . $httpResponse);
    #logmessage("<<Sent\n");
    
    
    #Read file from disk and send in N byte chunks.
    #we do this to conserve memory instead of loading/sending the whole file at once.
    my $chunkSize=1024;
    my $totalBytesSent=0;
    my $buffer;
    (open inFile, $fileName) || die logmessage("Can't open $fileName for reading: $!\n");     
    binmode inFile;

    #logmessage("Sending file chunks: $fileSize bytes, in $chunkSize chunks. ");
    while (read(inFile, $buffer, $chunkSize)) {
        # send the chunkLen in hex, the buffer bytes, and crlf
        my $chunkLen = sprintf("%x",length($buffer)); 
	logmessage($chunkLen . "\r\n\r\n");
        $sock->send($chunkLen . "\r\n" . $buffer . "\r\n");
        $totalBytesSent += length($buffer);
        #logmessage(".");
    }
   logmessage("0\r\n\r\n");
    $sock->send("0\r\n\r\n");
    close (inFile);

    logmessage("File Sent: $totalBytesSent bytes, ".(time()-$startTime)." seconds\n");
   
    # receive response
    #ICAP responses contain an ICAP header and possible body.
    #empty line indicates the end of the ICAP header.
    #we don't need any additional response body
    my $icapResponse; #entire response body
    my $icapStatus="000"; #ICAP status code
    my $resHdrSize;
    my $resBodySize;
    my $httpHdr;
    my $httpBody;
    
    while (my $line=<$sock>) {
    last if ($line eq "\r\n");
	
	logmessage("!!!!!!!!!!!!!:".$line);
        $icapResponse .= $line;
        
        #found ICAP response
        if($line =~ /^ICAP\/1.0 (\d+)/) {
            $icapStatus = $1;
            logmessage("ICAP Status Code found: $icapStatus\n");
        }
        #if there is a res-header and res-body, then there is more data after the icapResponse.        
        #res-body is the starting position of the httpHdr.        
        if( $line =~ /^Encapsulated: .*res-hdr=(\d+).*$/) {
            $resHdrSize=$1; 
            logmessage("Response Headers found: resHdrSize:$resHdrSize\n");
        }
      if( $line =~ /^Encapsulated: .*res-body=(\d+).*$/) {           
          $resBodySize=$1;
          logmessage("Response Body found: resBodySize:$resBodySize\n");
        }
    }
    logmessage("Read icapResponse: ".($icapResponse =~ tr/\n// )." lines\n");
    logmessage("icapResponse>>\n$icapResponse<<icapResponse\n");

    #If there is an http response, then more data needs to be read.
    #capture it in $httpHdr
    if ((defined $resHdrSize) and (defined $resBodySize)) {
        while (my $line=<$sock> ) {
        last if ($line eq "\r\n");      
            $httpHdr .= $line;        
        }
        logmessage("Read httpHdr: ".($httpHdr =~ tr/\n// )." lines\n");
        #logmessage("httpHdr>>\n$httpHdr<<httpHdr\n");
    }
    
    #if there is a res-body, we should be at the chunked byte size right here.
    if(defined $resBodySize && $resBodySize ne "0") {
        $/="\r\n"; #make this chomp the \r\n
        chomp(my $resBodySizeHex = <$sock>) || die logmessage("Cannot read resBodySize\n");
        $resBodySize = hex("$resBodySizeHex");
        logmessage("resBodySize:$resBodySize (0x$resBodySizeHex)\n" );
        
        #should be postioned at the top of the chunk.
        #read resBodySize number of bytes 
        #technically, according to ICAP, this is the modified file.
        #it could actually be binary being returned if 204s are not being used, 
        #however, MWG normally sends an html/text block message.

        read($sock, $httpBody, $resBodySize);

        #the only thing left in the buffer should be a \r\n0\r\n\r\n that we can discard
        while (my $line=<$sock>) {
        last if ($line eq "0\r\n");
        }
        logmessage("Read httpBody: ".($httpBody =~ tr/\n// )." lines\n");
        #logmessage("httpBody>>\n$httpBody<<httpBody\n");
    }
    
    # clean up socket 
    close($sock);

    logmessage("Socket Closed, responses received: ".(time()-$startTime)." seconds\n");



    # At this point we should have the reponses we can parse and process
    # $icapResponse; #entire ICAP response header
    # this can contain ICAP X-headers: that contain useful information
    
    # $icapStatus; #ICAP status code
    # Contrary to popular beilief, ICAP does scan for virus or allow or block content.
    # It simply "modifies" content.
    # If the content is Not Modified (204), it usually implies it's allowed.
    # If the contet is Modified (200), it has changed in some way that is different than the that was content sent.
    # Most "Modified" content is the return of a block page, indicating the content was blocked.
    # In its simplest form, 200=Blocked, 204=Allowed, anything else is an error or undefined.
    
    # $httpHdr; header of the HTTP Response
    # if the content was modified, an HTTP header is usually returned. This can contain usful information.
    
    # $httpBody; body of the HTTP Response
    # this is normally a block page message that is displayed in the client.
    
    #In it's simplest form, this should be enough.
    # Default allow unless explicitly blocked
    my $scanResult="Allowed";
    if ($icapStatus eq "200") { $scanResult = "Blocked"; }
    
    # you can also reverse the logic and block it by default unless you get a 204.
    # my $scanResult="Blocked";
    #if($icapStatus eq "204") { $scanResult = "Allowed"; }
    
    
    return $scanResult;
}


=begin comment Example of an ICAP Connection on a blocked file.

RESPMOD icap://192.168.2.231:1344/RESPMOD ICAP/1.0\r\n
Allow: 204\r\n
Connection: close\r\n
Host: 192.168.2.231\r\n
X-Client-IP: 192.168.1.2\r\n
Encapsulated: req-hdr=0, res-hdr=84, res-body=150\r\n
\r\n
GET http://192.168.1.2/1352779395/Artemis-Medium.zip HTTP/1.1\r\n
Host: 192.168.1.2\r\n
\r\n
HTTP/1.1 200 OK\r\n
Transfer-Encoding: chunked\r\n
Content-Length: 0\r\n
\r\n
211\r\n
...........\r\n
(529 bytes of binary data sent)\r\n
...........\r\n
0\r\n
\r\n
ICAP/1.0 200 OK\r\n
ISTag: "00001362-1.49.234-00006894"\r\n
Encapsulated: res-hdr=0, res-body=160\r\n
X-Scan-Stop: 2012-11-12 23:03:12\r\n
X-Media-Type: application/zip\r\n
X-Scan-Start: 2012-11-12 23:03:12\r\n
X-Virus-Name: McAfeeGW: Artemis!603F5BE29E9E, Avira: TR/Gendal.1024.J\r\n
X-Block-Reason: Malware found\r\n
X-Scan-Elapsed: 0\r\n
X-WWBlockResult: 80\r\n
\r\n
HTTP/1.1 403 VirusFound\r\n
Via: 1.1 192.168.2.231 (McAfee Web Gateway 7.2.0.3.0.13935)\r\n
Content-Type: text/html\r\n
Cache-Control: no-cache\r\n
Content-Length: 4022\r\n
\r\n
FB6\r\n
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">\r\n
<!--\r\n
  Message.TemplateName: VirusFound\r\n
  Message.Language: \r\n
  Fallback templates\r\n
-->\r\n
<html>\r\n
(4022 bytes of html body received)\r\n
</html>\r\n
0\r\n
\r\n
=end comment
=cut
