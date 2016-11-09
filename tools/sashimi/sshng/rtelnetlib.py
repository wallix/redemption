""" a replacement for telnetlib.py """

from __future__ import generators
# python imports
import socket, zlib
import time, re
# a little code to make this work in python 2.2
try:
  import itertools
  takewhile = itertools.takewhile
except ImportError:
    def takewhile(predicate, iterable):
        for x in iterable:
            if predicate(x):
                yield x
            else:
                break
# telnetlib exports squat by default, import some telnet negotiation constants
from telnetlib import (
        IAC,
        DONT,
        DO,
        WONT,
        WILL,
        SE,
        NOP,
        GA,
        SGA,
        SB,
        ECHO,
        EOR,
        AYT,
        NAWS,
        TTYPE
    )
TELOPT_EOR = chr(239)
# a couple standard ones used in TTYPE
SEND = chr(1)
IS = chr(0)
# and define some non-official ones
MCCP2 = chr(86)  # Mud Compression Protocol, v2
MCCP1 = chr(85)  # Mud Compression Protocol, v1 (broken and not supported here)
MSP = chr(90) # Mud Sound Protocol
MXP = chr(91) # Mud eXtension Protocol

# nego char to text
nego_names = {
        IAC:'IAC',
        DONT:'DONT',
        DO:'DO',
        WILL:'WILL',
        WONT:'WONT',
        SE:'SE',
        NOP:'NOP',
        GA:'GA',
        SB:'SB',
        ECHO:'ECHO',
        EOR:'EOR',
        MCCP2:'MCCP2',
        MCCP1:'MCCP1',
        AYT:'AYT',
        NAWS:'NAWS',
        TTYPE:'TTYPE',
        MSP:'MSP',
        MXP:'MXP',
        TELOPT_EOR:'TELOPT_EOR'
    }
    
def trans(val):
  try:
    return nego_names[val]
  except:
    return repr(val)

# exceptions used in negotiations
class NegotiateException(Exception): pass # base class
class NegotiateRemove(NegotiateException): pass # remove this handler, we're done
class NegotiateReply(NegotiateException):
  """ respond to the server with str(self) """
  def __init__(self, *responses):
    self.parts = responses
    return
  def __str__(self):
    return ''.join(self.parts)

class ReadException(Exception): pass
class Prompt(ReadException): pass
class Timeout(ReadException, socket.timeout): pass

def IAC_escape(data):
  """ double up any IACs in the string """
  return data.replace(IAC, IAC+IAC)

class Telnet(object):
  def __init__(self, host=None, port=23, **opts):
    """ Connect a TCP/IP socket to host:port
        If host is None you must call open() later to connect to a host.
        options:
        The 'recv_timeout' option will switch to non-blocking IO with that timeout
        The 'recv_size' option will ask to read that much data at a time from the socket.
    """
    # public members
    self.shutdown = 0
    self.recv_size = opts.get('recv_size', 50)
    self.buffer = ''
    self.can_negotiate = 0 # advisory, true after the server has initiated some negotiation
    self.handlers = {}
    self.handlers['default'] = will_nego #refuse_nego # just say "No" (actually say DONT or WONT)
    self.prompt_on_timeout = 0
    
    # private members
    self._socket = None
    self._recv_timeout = opts.get('recv_timeout', None)
    self._raise_soon = []

    # prepare our queue runner
    self._parse_buffer_iter = self._setup_buffer_iter()

    if (host is not None):
      self.open(host, port)
    return

  # make self.recv_timeout a property
  def _set_recv_timeout(self, timeout):
    self._recv_timeout = timeout
    if (self._socket):
      self._socket.settimeout(timeout)
    return
  def _get_recv_timeout(self):
    return self._recv_timeout
  recv_timeout = property(_get_recv_timeout, _set_recv_timeout)

  def fileno(self):
    return self._socket.fileno()

  def do_shutdown(self):
    self.shutdown = 1
    return

  def close(self):
    self.do_shutdown()
    if self._socket is not None:
        self._socket.close()

  def open(self, host, port=23):
    """Connect to a host.
       The optional second argument is the port number, which defaults to the
       standard telnet port (23).
       Invalid if we are already connected.
    """
    if (self._socket):
      raise ValueError("Already connected")
    
    self.host = host
    self.port = port
    matching_hosts = socket.getaddrinfo(host, port, 0, socket.SOCK_STREAM)
    if (not matching_hosts):
      raise socket.error("getaddrinfo(%s,%s) returned an empty list" % (repr(host), repr(port)))

    last_error = None
    for (af, socktype, proto, cannonname, sa) in matching_hosts:
      try:
        self._socket = socket.socket(af, socktype, proto)
        self._socket.connect(sa)
        break
      except socket.error, msg:
        last_error = socket.error(msg)
        if self._socket:
          self._socket.close()
          self._socket = None
    else: # all errors, 'break' was never reached
      raise ValueError("%s" % last_error)
    
    if (self.recv_timeout):
      self._socket.settimeout(self.recv_timeout)
    return

  def write(self, data):
    """ writes 'data' to socket.
        Will escape any IACs [chr(255)] in the stream.
        Block if the socket is not ready for writing.
        Raises EOFError if the socket is closed.
    """
    self.rawwrite(IAC_escape(data))
    return len(data)

  def write_filter(self, data):
    """ Called from write() and write_noblock() to give subclasses the chance
        to change the data.  Return value should be the changed data to write.
        This is called before any escaping of the data.
    """
    raise NotImplementedError()

  def write_socket_filter(self, data):
    """ like write_filter() but called _after_ the data is escaped, so data is
        the raw data that will be written to the socket
    """
    raise NotImplementedError()
  
  def rawwrite(self, data):
    """ Write to the socket like write(), but do no escaping on the data """
    try:
      self._socket.sendall(data)
    except (Exception), e:
      self.do_shutdown()
    return

  def read(self):
    """Return data from the socket.
       raises Timeout error if 'timeout' was specified and there is nothing to read
       after 'timeout' seconds.
       raises Prompt error if the 'prompt_on_timeout' option is set, and may raise
       a Prompt error from a user defined telnet negotiation handler.
       raises EOFError if the socket is closed.
    """
    try:
      return self._read()
    except socket.timeout, e:
      if (self.prompt_on_timeout):
        raise Prompt()
      else:
        raise Timeout()
    except Prompt, e: # catch and reraise
      raise e
    except EOFError, e:
      self.do_shutdown()
      raise e
    except Exception, e:
      import sys, traceback
      traceback.print_exception(type(e), e, sys.exc_traceback)
      return

  def read_noblock(self):
    """ a read() that will return immediately.  If there is no data ready returns
        an empty string.  Raises EOFError if the socket is closed.
        May raise a Prompt error as read().
    """
    # I don't think we have to do half these calls
    orig_recv_timeout = self._socket.gettimeout()
    self._socket.setblocking(0)
    data = self._read()
    self._socket.setblocking(1)
    self._socket.settimeout(orig_recv_timeout)
    return data

  def read_filter(self, data):
    """ Called from read() and read_noblock() to give subclasses the chance
        to change the data.  The data argument should be changed (or not)
        and returned from this function.
    """
    return data

  def read_socket_filter(self, data):
    """ like read_filter() but data is the raw data from the socket _before processing_ """
    return data

  def _read(self):
    if (self._raise_soon):
      raise self._raise_soon.pop()
    
    data = self._socket.recv(self.recv_size) # may raise socket.timeout
    if (not data):
      self.do_shutdown()
      raise EOFError()
    self.buffer += self.read_socket_filter(data)
    data = ''.join(list(itertools.takewhile(lambda x:x is not None, self._parse_buffer_iter)))
    return self.read_filter(data)

  def expect(self, vals, timeout = 60):
    indices = range(len(vals))
    for i in indices:
        if not hasattr(vals[i], "search"):
            vals[i] = re.compile(vals[i])
    data = ''
    init_time = time.time()
    while 1:
        try:
            data += self.read()
        except EOFError:
            return (-1, None, None)
        for i in indices:
            m = vals[i].search(data)
            if m:
                e = m.end()
                text = data[:e]
                return (i, m, text)
        if time.time() - init_time > timeout:
            return (-1, None, None)

  def handle_nego(self, cmd, option, sb_data):
    if (option in self.handlers):
      handler_key = option
    elif (cmd in self.handlers):
      handler_key = cmd
    elif ('default' in self.handlers):
      handler_key = 'default'
    else: # nothing to do
      return

    if (option not in [None, ECHO]): # a 'real' option, the server has shown in knows something about negotiation
      self.can_negotiate = 1 # set our advisory flag
    
    handler = self.handlers[handler_key]
    try:
      handler(cmd, option, sb_data)
    except NegotiateRemove: # remove this handler
      del self.handlers[handler_key]
    except NegotiateReply, e:
      self.rawwrite(str(e))
    except Prompt, e:
      self._raise_soon.append(e)
    return

  def _setup_buffer_iter(self):
    def clean_data(data):
      """ the old telnetlib does this, I'm not sure why """
      return data.replace(chr(0),'').replace('\021', '')

    while (not self.shutdown):
      if (not self.buffer): # nothing to parse, yield None as a pause indication (DEBUG: is this possible?)
        yield None
        continue
      
      iac_ind = self.buffer.find(IAC)
      if (iac_ind == -1): # typical case, no telnet negotiation
        data = self.buffer
        self.buffer = ''
        yield clean_data(data)
        continue
      # IAC hiding in there
      if (iac_ind): # not at zero, yield the data first
        data = self.buffer[:iac_ind]
        self.buffer = self.buffer[iac_ind:]
        yield clean_data(data)
        continue
      # okie, we have an IAC at position zero.  do some work.
      # we index blindly through the array, if we get an IndexError we have
      # a partial IAC sequence, just yield None and we'll try it again
      # the next time when we have more data
      try:
        sb_data = None
        cmd = self.buffer[1]
        i = 2 # just after the command
        if (cmd in [SB, DO, DONT, WILL, WONT]):
          option = self.buffer[i]
          i += 1 # just after the option
        elif (cmd == IAC):
          self.buffer = self.buffer[i:]
          yield IAC # escaped IAC
          continue
        else:
          option = None
        if (cmd == SB):
          # this is slightly incorrect, we treat (bad) IAC sequences as data until the terminating IAC SE
          # for instance, a random IAC+NOP will be included in the SB..SE data.
          # err, check the RFC, I think the standard telnetlib.py treats everything as data, period. even including IAC+IAC
          looking_for_se = 1
          while (looking_for_se):
            if (self.buffer[i] == IAC):
              if (self.buffer[i+1] == IAC): # escaped IAC
                i += 2 # increment past IAC+IAC
              elif (self.buffer[i+1] == SE):
                i += 2 # increment past IAC+SE
                looking_for_se = 0
              else:
                i += 1
            else:
              i += 1
          sb_data = self.buffer[3:i-2] # IAC SB OPTION <data> IAC SE
        self.buffer = self.buffer[i:] # just past IAC stuff
        self.handle_nego(cmd, option, sb_data)
        continue
      except IndexError: # we didn't have the full IAC sequence, we'll try again later
        yield None
    # while (not self.shutdown)
    return

class NegoECHO(object):
  """ Negotiates ECHO.
      The 'callback' option will be called with a boolean on/off when the server asks
      to change the current ECHO
  """
  def __init__(self, **opts):
    self.echo = 1 # default to on
    self.callback = opts.get('callback', None)
    return

  def do_nego(self, command, option, sb_data):
    if (command == WILL):
      self.echo = 0
      reply = IAC+DO+ECHO
    elif (command == WONT):
      self.echo = 1
      reply = IAC+DONT+ECHO
    else:
      return
    if (self.callback):
      self.callback(self.echo)
    raise NegotiateReply(reply)
  __call__ = do_nego # make the module callable by aliasing do_nego

class NegoMCCP(object):
  """ Negotaite MCCP.
      This handler only responds to MCCP2, if you change the default handler from saying
      'NO!' to everything (including MCCP1) insert a refuse_nego handler for MCCP1.
  """
  def __init__(self, telnet_ob):
    self.telnet_ob = telnet_ob # master telnet object
    return
  
  def do_nego(self, command, option, sb_data):
    """ handle the telnet negotiation """
    if (command == WILL):
      raise NegotiateReply(IAC,DO,MCCP2)
    if (command == SB and option == MCCP2):
      # the server has said _every byte_ from now on will be compressed
      # create a decompression object and wrap the telnet object's read_socket_filter method
      ungzip_ob = zlib.decompressobj(15)
      existing_read_socket_filter = self.telnet_ob.read_socket_filter
      def mccp_read_socket_filter(data):
        # give the original func a chance to munge the data
        data = existing_read_socket_filter(data)
        # now do our work
        return ungzip_ob.decompress(data)
      
      # filter the current buffer, which is all data after the SB reply
      self.telnet_ob.buffer = ungzip_ob.decompress(self.telnet_ob.buffer)
      # replace the existing read_socket_filter with our version
      setattr(self.telnet_ob, 'read_socket_filter', mccp_read_socket_filter)
      # we can't be un-negotiated, so take us out of the loop
      raise NegotiateRemove()
    return
  __call__ = do_nego # make the module callable by aliasing do_nego

class NegoNAWS(object):
  def __init__(self, socket_write = None, **opts):
    self.width = opts.get('width', 80)
    self.height = opts.get('height', 40)
    self.seen_do_naws = 0 # if true we can send updates any time
    self.i_said_will_naws = 0 # true if we sent a WILL NAWS already
    self.never_again = 0 # we have been told to stop
    self.socket_write = socket_write
    return

  def make_sb_clause(self):
    sb_start = IAC + SB + NAWS
    w = chr(int(self.width/256)) + chr(int(self.width%256))
    h = chr(int(self.height/256)) + chr((self.height%256))
    sb_data = IAC_escape(w+h)
    sb_end = IAC + SE
    return '%s%s%s' % (sb_start, sb_data, sb_end)

  def do_nego(self, command, option, sb_data):
    if (command == DONT): # the server is telling us to not bother
      self.never_again = 1
      raise NegotiateRemove()
    if (command == DO):
      self.seen_do_naws = 1
      reply_parts = []
      if (not self.i_said_will_naws): # he initiated, we haven't said yes yet
        reply_parts.append(IAC + WILL + NAWS)
        self.i_said_will_naws

      # either way, always send the data
      reply_parts.append(self.make_sb_clause())
      raise NegotiateReply(*reply_parts)
    return
  __call__ = do_nego # alias to make the module callable

  def update_size(self, width, height):
    """ update our size state """
    changed = 0
    if (width != self.width):
      changed = 1
      self.width = width
    if (height != self.height):
      changed = 1
      self.height = height
    if (changed and self.socket_write):
      if (self.seen_do_naws): # just write the update
        self.socket_write(self.make_sb_clause())
      else: # initiate negotiation
        self.suggest_naws()
    return
  
  def suggest_naws(self):
    if (self.never_again): # we have already taken ourselves out of the nego loop
      return
    if (not self.socket_write): # we don't know how to write to the socket
      return
    self.my_suggestion = 1
    self.socket_write(IAC+WILL+NAWS)
    return

class NegoTTYPE(object):
  def __init__(self, term_type_list):
    self.ttypes = []
    # we reply with each term type in order, and the last one TWICE to let the server
    # know we have used up all the options
    for (ttype) in term_type_list:
      if (not self.ttypes or ttype != self.ttypes[-1]): # remove consecutive dups in the original list
        self.ttypes.append(ttype)
    # duplicate the last one twice
    self.ttypes.append(self.ttypes[-1])
    self.ttype_ind = -1 # where we are in the list when talking to the server
    return
  
  def do_nego(self, command, option, sb_data):
    """ a method suitable for the handlers interface """
    if (command == DO):
      raise NegotiateReply(IAC, WILL, TTYPE)
    if (sb_data):
      self.ttype_ind += 1
      if (self.ttype_ind >= len(self.ttypes)):
        self.ttype_ind = 0
      sb_reply = IAC_escape(self.ttypes[self.ttype_ind])
      raise NegotiateReply(IAC, SB, TTYPE, IS, sb_reply, IAC, SE)
    return
  __call__ = do_nego # make the object callable by aliasing do_nego

def raise_prompt(*args):
  raise Prompt()

def will_nego(command, option, sb_data):
  if (command == WILL): # server asks if it can send 'option' (like EOR)
    raise NegotiateReply(IAC, DO, option) # yes please!
  elif (command == DO): # server asks us to start sending 'option'
    raise NegotiateReply(IAC, WONT, option) # reply 'not sending'
  elif (command in [WONT, DONT]): pass
  
def refuse_nego(command, option, sb_data):
  """ A simple handler that always replies 'I can't do that, please don't ask again' """
  if (option is None): # no option, so we don't know what to refuse to do
    return
  
  if (command in [WILL, WONT]):
    raise NegotiateReply(IAC, DONT, option)
  elif (command in [DO, DONT]):
    raise NegotiateReply(IAC, WONT, option)
  else: pass # got an option, but with an odd command.  just stay silent
  return
