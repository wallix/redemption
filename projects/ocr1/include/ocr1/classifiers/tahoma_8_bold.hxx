if (count < 34) {
  if (count < 20) {
    if (count < 12) {
      if (count < 8) {
        if (count == 4) {
          if (width == 2) {
            return ".";
          } else /* NOLINT */ if (width == 3) {
            return "`";
          } else /* NOLINT */ if (width == 4) {
            return "-";
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 6) {
          return "'";
        } else /* NOLINT */ if (count == 7) {
          if (width == 2) {
            return ",";
          } else /* NOLINT */ if (width == 7) {
            return "_";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else /* NOLINT */ {
        if (count == 8) {
          if (width == 2) {
            return ":";
          } else /* NOLINT */ if (width == 4) {
            return "°";
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 10) {
          return "—";
        } else /* NOLINT */ if (count == 11) {
          if (width == 2) {
            return ";";
          } else /* NOLINT */ if (width == 5) {
            if (height == 11) {
              if (pixel(0) == 1) {
                return "\\";
              } else /* NOLINT */ if (pixel(0) == 0) {
                return "/";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      }
    } else /* NOLINT */ {
      if (count < 15) {
        if (count == 12) {
          return "~";
        } else /* NOLINT */ if (count == 13) {
          if (width == 7) {
            if (height == 7) {
              if (pixel(0) == 1) {
                return ">";
              } else /* NOLINT */ if (pixel(0) == 0) {
                if (pixel(3) == 1) {
                  return "+";
                } else /* NOLINT */ if (pixel(3) == 0) {
                  return "<";
                } else /* NOLINT */ {}
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 14) {
          if (width == 2) {
            if (height == 8) {
              if (pixel(2) == 1) {
                return "!";
              } else /* NOLINT */ if (pixel(2) == 0) {
                return "i";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ if (width == 7) {
            return "=";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else /* NOLINT */ {
        if (count == 15) {
          return "r";
        } else /* NOLINT */ if (count == 16) {
          if (width == 4) {
            return "î";
          } else /* NOLINT */ if (width == 5) {
            return "c";
          } else /* NOLINT */ if (width == 6) {
            if (height == 5) {
              return "*";
            } else /* NOLINT */ if (height == 6) {
              return "¤";
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 18) {
          if (width == 2) {
            return "l";
          } else /* NOLINT */ if (width == 5) {
            return "z";
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 19) {
          if (width == 3) {
            return "j";
          } else /* NOLINT */ if (width == 4) {
            if (height == 8) {
              if (pixel(0) == 1) {
                return "t";
              } else /* NOLINT */ if (pixel(0) == 0) {
                return "1";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ if (width == 5) {
            if (height == 8) {
              if (pixel(0) == 1) {
                return "L";
              } else /* NOLINT */ if (pixel(0) == 0) {
                return "ç";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      }
    }
  } else /* NOLINT */ {
    if (count < 27) {
      if (count < 24) {
        if (count == 20) {
          if (width == 4) {
            if (height == 8) {
              return "I";
            } else /* NOLINT */ if (height == 9) {
              return "f";
            } else /* NOLINT */ {}
          } else /* NOLINT */ if (width == 5) {
            if (height == 6) {
              return "s";
            } else /* NOLINT */ if (height == 8) {
              return "J";
            } else /* NOLINT */ {}
          } else /* NOLINT */ if (width == 6) {
            if (height == 6) {
              return "v";
            } else /* NOLINT */ if (height == 8) {
              if (pixel(8) == 1) {
                return "T";
              } else /* NOLINT */ if (pixel(8) == 0) {
                return "7";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 22) {
          if (width == 4) {
            if (height == 11) {
              if (pixel(0) == 1) {
                return ")";
              } else /* NOLINT */ if (pixel(0) == 0) {
                return "(";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ if (width == 5) {
            return "F";
          } else /* NOLINT */ if (width == 6) {
            if (height == 8) {
              if (pixel(1) == 1) {
                return "C";
              } else /* NOLINT */ if (pixel(1) == 0) {
                return "4";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 23) {
          return "e";
        } else /* NOLINT */ {}
      } else /* NOLINT */ {
        if (count == 24) {
          if (width == 6) {
            if (height == 6) {
              if (pixel(0) == 1) {
                return "x";
              } else /* NOLINT */ if (pixel(0) == 0) {
                if (pixel(6) == 1) {
                  return "o";
                } else /* NOLINT */ if (pixel(6) == 0) {
                  return "a";
                } else /* NOLINT */ {}
              } else /* NOLINT */ {}
            } else /* NOLINT */ if (height == 8) {
              if (pixel(0) == 1) {
                if (pixel(37) == 1) {
                  return "y";
                } else /* NOLINT */ if (pixel(37) == 0) {
                  return "Y";
                } else /* NOLINT */ {}
              } else /* NOLINT */ if (pixel(0) == 0) {
                return "2";
              } else /* NOLINT */ {}
            } else /* NOLINT */ if (height == 10) {
              return "Ï";
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 25) {
          if (width == 5) {
            return "E";
          } else /* NOLINT */ if (width == 6) {
            if (height == 6) {
              if (pixel(2) == 1) {
                return "n";
              } else /* NOLINT */ if (pixel(2) == 0) {
                return "u";
              } else /* NOLINT */ {}
            } else /* NOLINT */ if (height == 8) {
              if (pixel(1) == 1) {
                if (pixel(5) == 1) {
                  return "5";
                } else /* NOLINT */ if (pixel(5) == 0) {
                  return "3";
                } else /* NOLINT */ {}
              } else /* NOLINT */ if (pixel(1) == 0) {
                return "£";
              } else /* NOLINT */ {}
            } else /* NOLINT */ if (height == 11) {
              return "}";
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 26) {
          if (width == 4) {
            if (height == 11) {
              if (pixel(4) == 1) {
                return "[";
              } else /* NOLINT */ if (pixel(4) == 0) {
                return "]";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      }
    } else /* NOLINT */ {
      if (count < 30) {
        if (count == 27) {
          if (width == 6) {
            if (height == 8) {
              return "ë";
            } else /* NOLINT */ if (height == 9) {
              if (pixel(1) == 1) {
                return "è";
              } else /* NOLINT */ if (pixel(1) == 0) {
                return "é";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 28) {
          if (width == 6) {
            if (height == 8) {
              if (pixel(4) == 1) {
                if (pixel(5) == 1) {
                  if (pixel(13) == 1) {
                    if (pixel(3) == 1) {
                      return "S";
                    } else /* NOLINT */ if (pixel(3) == 0) {
                      if (pixel(0) == 1) {
                        if (pixel(6) == 1) {
                          if (pixel(12) == 1) {
                            return "V";
                          } else /* NOLINT */ if (pixel(12) == 0) {
                            return "X";
                          } else /* NOLINT */ {}
                        } else /* NOLINT */ if (pixel(6) == 0) {
                          return "ö";
                        } else /* NOLINT */ {}
                      } else /* NOLINT */ if (pixel(0) == 0) {
                        return "ä";
                      } else /* NOLINT */ {}
                    } else /* NOLINT */ {}
                  } else /* NOLINT */ if (pixel(13) == 0) {
                    return "Z";
                  } else /* NOLINT */ {}
                } else /* NOLINT */ if (pixel(5) == 0) {
                  return "P";
                } else /* NOLINT */ {}
              } else /* NOLINT */ if (pixel(4) == 0) {
                return "N";
              } else /* NOLINT */ {}
            } else /* NOLINT */ if (height == 9) {
              return "à";
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 29) {
          if (width == 5) {
            return "Ë";
          } else /* NOLINT */ if (width == 6) {
            if (height == 8) {
              if (pixel(3) == 1) {
                if (pixel(10) == 1) {
                  return "9";
                } else /* NOLINT */ if (pixel(10) == 0) {
                  return "6";
                } else /* NOLINT */ {}
              } else /* NOLINT */ if (pixel(3) == 0) {
                return "ü";
              } else /* NOLINT */ {}
            } else /* NOLINT */ if (height == 9) {
              return "ù";
            } else /* NOLINT */ {}
          } else /* NOLINT */ if (width == 7) {
            return "€";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else /* NOLINT */ {
        if (count == 30) {
          if (width == 6) {
            if (height == 8) {
              if (pixel(0) == 1) {
                if (pixel(2) == 1) {
                  return "p";
                } else /* NOLINT */ if (pixel(2) == 0) {
                  if (pixel(9) == 1) {
                    return "K";
                  } else /* NOLINT */ if (pixel(9) == 0) {
                    return "µ";
                  } else /* NOLINT */ {}
                } else /* NOLINT */ {}
              } else /* NOLINT */ if (pixel(0) == 0) {
                return "q";
              } else /* NOLINT */ {}
            } else /* NOLINT */ if (height == 9) {
              if (pixel(0) == 1) {
                return "k";
              } else /* NOLINT */ if (pixel(0) == 0) {
                return "ê";
              } else /* NOLINT */ {}
            } else /* NOLINT */ if (height == 11) {
              return "$";
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 31) {
          if (width == 6) {
            if (height == 9) {
              if (pixel(0) == 1) {
                return "h";
              } else /* NOLINT */ if (pixel(0) == 0) {
                if (pixel(24) == 1) {
                  return "ô";
                } else /* NOLINT */ if (pixel(24) == 0) {
                  return "â";
                } else /* NOLINT */ {}
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ if (width == 7) {
            return "G";
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 32) {
          if (width == 6) {
            if (height == 8) {
              if (pixel(5) == 1) {
                return "g";
              } else /* NOLINT */ if (pixel(5) == 0) {
                if (pixel(18) == 1) {
                  return "0";
                } else /* NOLINT */ if (pixel(18) == 0) {
                  return "8";
                } else /* NOLINT */ {}
              } else /* NOLINT */ {}
            } else /* NOLINT */ if (height == 9) {
              if (pixel(0) == 1) {
                return "b";
              } else /* NOLINT */ if (pixel(0) == 0) {
                if (pixel(2) == 1) {
                  return "û";
                } else /* NOLINT */ if (pixel(2) == 0) {
                  return "d";
                } else /* NOLINT */ {}
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 33) {
          if (width == 7) {
            if (height == 8) {
              if (pixel(0) == 1) {
                return "U";
              } else /* NOLINT */ if (pixel(0) == 0) {
                return "A";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      }
    }
  }
} else /* NOLINT */ {
  if (count < 58) {
    if (count < 42) {
      if (count < 38) {
        if (count == 34) {
          if (width == 6) {
            if (height == 9) {
              return "fi";
            } else /* NOLINT */ if (height == 10) {
              return "§";
            } else /* NOLINT */ {}
          } else /* NOLINT */ if (width == 7) {
            if (height == 8) {
              if (pixel(0) == 1) {
                if (pixel(18) == 1) {
                  return "R";
                } else /* NOLINT */ if (pixel(18) == 0) {
                  return "D";
                } else /* NOLINT */ {}
              } else /* NOLINT */ if (pixel(0) == 0) {
                return "O";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ if (width == 8) {
            return "w";
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 35) {
          if (width == 6) {
            return "B";
          } else /* NOLINT */ if (width == 7) {
            return "H";
          } else /* NOLINT */ if (width == 8) {
            if (height == 8) {
              return "&";
            } else /* NOLINT */ if (height == 9) {
              return "ît";
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 37) {
          if (width == 7) {
            if (height == 10) {
              if (pixel(14) == 1) {
                return "Ü";
              } else /* NOLINT */ if (pixel(14) == 0) {
                return "Ä";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else /* NOLINT */ {
        if (count == 38) {
          if (width == 6) {
            return "fl";
          } else /* NOLINT */ if (width == 7) {
            return "Ö";
          } else /* NOLINT */ if (width == 8) {
            return "#";
          } else /* NOLINT */ if (width == 10) {
            return "m";
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 39) {
          if (width == 7) {
            return "Q";
          } else /* NOLINT */ if (width == 8) {
            return "ft";
          } else /* NOLINT */ if (width == 9) {
            return "M";
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 40) {
          if (width == 7) {
            return "Â";
          } else /* NOLINT */ if (width == 8) {
            return "ff";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      }
    } else /* NOLINT */ {
      if (count < 50) {
        if (count == 42) {
          return "@";
        } else /* NOLINT */ if (count == 44) {
          return "%";
        } else /* NOLINT */ if (count == 46) {
          return "W";
        } else /* NOLINT */ {}
      } else /* NOLINT */ {
        if (count == 50) {
          return "fk";
        } else /* NOLINT */ if (count == 51) {
          return "fh";
        } else /* NOLINT */ if (count == 52) {
          return "fb";
        } else /* NOLINT */ if (count == 54) {
          return "ffi";
        } else /* NOLINT */ {}
      }
    }
  } else /* NOLINT */ {
    if (count < 74) {
      if (count < 70) {
        if (count == 58) {
          return "ffl";
        } else /* NOLINT */ if (count == 59) {
          return "fft";
        } else /* NOLINT */ if (count == 60) {
          return "fff";
        } else /* NOLINT */ {}
      } else /* NOLINT */ {
        if (count == 70) {
          return "ffk";
        } else /* NOLINT */ if (count == 71) {
          return "ffh";
        } else /* NOLINT */ if (count == 72) {
          return "ffb";
        } else /* NOLINT */ {}
      }
    } else /* NOLINT */ {
      if (count < 80) {
        if (count == 74) {
          return "fffi";
        } else /* NOLINT */ if (count == 78) {
          return "fffl";
        } else /* NOLINT */ if (count == 79) {
          return "ffft";
        } else /* NOLINT */ {}
      } else /* NOLINT */ {
        if (count == 80) {
          return "ffff";
        } else /* NOLINT */ if (count == 90) {
          return "fffk";
        } else /* NOLINT */ if (count == 91) {
          return "fffh";
        } else /* NOLINT */ if (count == 92) {
          return "fffb";
        } else /* NOLINT */ {}
      }
    }
  }
}
