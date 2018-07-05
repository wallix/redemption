if (count < 38) {
  if (count < 22) {
    if (count < 12) {
      if (count < 7) {
        if (count == 3) {
          return "-";
        } else if (count == 4) {
          if (width == 2) {
            return ".";
          } else if (width == 3) {
            return "`";
          } else /* NOLINT */ {}
        } else if (count == 6) {
          if (width == 2) {
            if (height == 3) {
              return "'";
            } else if (height == 4) {
              return ",";
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else {
        if (count == 7) {
          return "~";
        } else if (count == 8) {
          if (width == 2) {
            return ":";
          } else if (width == 3) {
            return "°";
          } else if (width == 8) {
            return "_";
          } else /* NOLINT */ {}
        } else if (count == 9) {
          return "+";
        } else if (count == 10) {
          return ";";
        } else /* NOLINT */ {}
      }
    } else {
      if (count < 18) {
        if (count == 12) {
          if (width == 5) {
            return ">";
          } else if (width == 6) {
            return "=";
          } else /* NOLINT */ {}
        } else if (count == 13) {
          return "|";
        } else if (count == 14) {
          return "<";
        } else if (count == 17) {
          return "r";
        } else /* NOLINT */ {}
      } else {
        if (count == 18) {
          if (width == 2) {
            return "!";
          } else if (width == 6) {
            return "*";
          } else /* NOLINT */ {}
        } else if (count == 19) {
          if (width == 3) {
            if (height == 10) {
              if (pixel(0) == 1) {
                return "ì";
              } else if (pixel(0) == 0) {
                return "i";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 4) {
            return "ï";
          } else if (width == 5) {
            if (height == 10) {
              if (pixel(0) == 1) {
                return "\\";
              } else if (pixel(0) == 0) {
                return "/";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 20) {
          if (width == 2) {
            return "I";
          } else if (width == 3) {
            return "l";
          } else if (width == 4) {
            return "t";
          } else if (width == 6) {
            return "¤";
          } else /* NOLINT */ {}
        } else if (count == 21) {
          return "s";
        } else /* NOLINT */ {}
      }
    }
  } else {
    if (count < 30) {
      if (count < 26) {
        if (count == 22) {
          if (width == 3) {
            return "(";
          } else if (width == 5) {
            return "î";
          } else if (width == 6) {
            if (height == 7) {
              if (pixel(0) == 1) {
                return "z";
              } else if (pixel(0) == 0) {
                return "c";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 23) {
          if (width == 3) {
            return ")";
          } else if (width == 5) {
            if (height == 10) {
              if (pixel(0) == 1) {
                return "?";
              } else if (pixel(0) == 0) {
                return "f";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 7) {
            return "v";
          } else /* NOLINT */ {}
        } else if (count == 24) {
          if (width == 3) {
            return "Ì";
          } else if (width == 4) {
            return "1";
          } else if (width == 6) {
            if (height == 10) {
              if (pixel(0) == 1) {
                if (pixel(2) == 1) {
                  return "7";
                } else if (pixel(2) == 0) {
                  return "L";
                } else /* NOLINT */ {}
              } else if (pixel(0) == 0) {
                return "J";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 25) {
          return "x";
        } else /* NOLINT */ {}
      } else {
        if (count == 26) {
          if (width == 4) {
            return "j";
          } else if (width == 8) {
            return "T";
          } else /* NOLINT */ {}
        } else if (count == 27) {
          if (width == 5) {
            return "Î";
          } else if (width == 6) {
            if (height == 10) {
              if (pixel(0) == 1) {
                return "F";
              } else if (pixel(0) == 0) {
                return "ç";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 28) {
          if (width == 5) {
            if (height == 10) {
              return "E";
            } else if (height == 13) {
              if (pixel(0) == 1) {
                return "}";
              } else if (pixel(0) == 0) {
                return "{";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 6) {
            if (height == 7) {
              if (pixel(0) == 1) {
                return "n";
              } else if (pixel(0) == 0) {
                if (pixel(6) == 1) {
                  if (pixel(20) == 1) {
                    return "e";
                  } else if (pixel(20) == 0) {
                    return "o";
                  } else /* NOLINT */ {}
                } else if (pixel(6) == 0) {
                  return "a";
                } else /* NOLINT */ {}
              } else /* NOLINT */ {}
            } else if (height == 10) {
              if (pixel(0) == 1) {
                return "Z";
              } else if (pixel(0) == 0) {
                return "£";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 7) {
            if (height == 10) {
              if (pixel(0) == 1) {
                return "y";
              } else if (pixel(0) == 0) {
                if (pixel(2) == 1) {
                  return "C";
                } else if (pixel(2) == 0) {
                  return "#";
                } else /* NOLINT */ {}
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 29) {
          return "u";
        } else /* NOLINT */ {}
      }
    } else {
      if (count < 34) {
        if (count == 30) {
          if (width == 4) {
            if (height == 13) {
              if (pixel(4) == 1) {
                return "[";
              } else if (pixel(4) == 0) {
                return "]";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 6) {
            if (height == 10) {
              if (pixel(26) == 1) {
                return "3";
              } else if (pixel(26) == 0) {
                return "2";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 7) {
            return "€";
          } else if (width == 8) {
            if (height == 9) {
              return "@";
            } else if (height == 10) {
              return "Y";
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 31) {
          return "4";
        } else if (count == 32) {
          if (width == 5) {
            if (height == 13) {
              if (pixel(0) == 1) {
                return "È";
              } else if (pixel(0) == 0) {
                return "É";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 6) {
            if (height == 10) {
              if (pixel(0) == 1) {
                return "5";
              } else if (pixel(0) == 0) {
                if (pixel(5) == 1) {
                  return "é";
                } else if (pixel(5) == 0) {
                  if (pixel(1) == 1) {
                    if (pixel(11) == 1) {
                      return "S";
                    } else if (pixel(11) == 0) {
                      if (pixel(3) == 1) {
                        return "9";
                      } else if (pixel(3) == 0) {
                        if (pixel(24) == 1) {
                          if (pixel(38) == 1) {
                            return "è";
                          } else if (pixel(38) == 0) {
                            return "ò";
                          } else /* NOLINT */ {}
                        } else if (pixel(24) == 0) {
                          return "à";
                        } else /* NOLINT */ {}
                      } else /* NOLINT */ {}
                    } else /* NOLINT */ {}
                  } else if (pixel(1) == 0) {
                    return "6";
                  } else /* NOLINT */ {}
                } else /* NOLINT */ {}
              } else /* NOLINT */ {}
            } else if (height == 12) {
              return "$";
            } else /* NOLINT */ {}
          } else if (width == 9) {
            return "%";
          } else /* NOLINT */ {}
        } else if (count == 33) {
          if (width == 6) {
            return "ù";
          } else if (width == 7) {
            return "V";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else {
        if (count == 34) {
          if (width == 5) {
            return "§";
          } else if (width == 6) {
            if (height == 10) {
              if (pixel(24) == 1) {
                return "õ";
              } else if (pixel(24) == 0) {
                return "ã";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 7) {
            return "A";
          } else /* NOLINT */ {}
        } else if (count == 35) {
          if (width == 5) {
            return "Ê";
          } else if (width == 6) {
            if (height == 10) {
              if (pixel(0) == 1) {
                return "h";
              } else if (pixel(0) == 0) {
                if (pixel(24) == 1) {
                  if (pixel(38) == 1) {
                    return "ê";
                  } else if (pixel(38) == 0) {
                    return "ô";
                  } else /* NOLINT */ {}
                } else if (pixel(24) == 0) {
                  return "â";
                } else /* NOLINT */ {}
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 7) {
            return "k";
          } else /* NOLINT */ {}
        } else if (count == 36) {
          if (width == 6) {
            if (height == 10) {
              if (pixel(4) == 1) {
                if (pixel(7) == 1) {
                  if (pixel(1) == 1) {
                    if (pixel(0) == 1) {
                      if (pixel(2) == 1) {
                        return "p";
                      } else if (pixel(2) == 0) {
                        if (pixel(12) == 1) {
                          return "µ";
                        } else if (pixel(12) == 0) {
                          if (pixel(24) == 1) {
                            if (pixel(38) == 1) {
                              return "ë";
                            } else if (pixel(38) == 0) {
                              return "ö";
                            } else /* NOLINT */ {}
                          } else if (pixel(24) == 0) {
                            return "ä";
                          } else /* NOLINT */ {}
                        } else /* NOLINT */ {}
                      } else /* NOLINT */ {}
                    } else if (pixel(0) == 0) {
                      return "q";
                    } else /* NOLINT */ {}
                  } else if (pixel(1) == 0) {
                    return "û";
                  } else /* NOLINT */ {}
                } else if (pixel(7) == 0) {
                  return "d";
                } else /* NOLINT */ {}
              } else if (pixel(4) == 0) {
                return "b";
              } else /* NOLINT */ {}
            } else if (height == 13) {
              return "Ë";
            } else /* NOLINT */ {}
          } else if (width == 7) {
            return "P";
          } else if (width == 8) {
            return "X";
          } else /* NOLINT */ {}
        } else if (count == 37) {
          if (width == 6) {
            return "ü";
          } else if (width == 8) {
            return "G";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      }
    }
  }
} else {
  if (count < 55) {
    if (count < 46) {
      if (count < 42) {
        if (count == 38) {
          if (width == 7) {
            if (height == 10) {
              return "0";
            } else if (height == 13) {
              return "À";
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 39) {
          if (width == 6) {
            return "8";
          } else if (width == 11) {
            return "w";
          } else /* NOLINT */ {}
        } else if (count == 40) {
          if (width == 7) {
            return "Ã";
          } else if (width == 8) {
            if (height == 10) {
              if (pixel(0) == 1) {
                return "U";
              } else if (pixel(0) == 0) {
                return "O";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 41) {
          if (width == 7) {
            if (height == 10) {
              return "g";
            } else if (height == 13) {
              return "Â";
            } else /* NOLINT */ {}
          } else if (width == 8) {
            return "ls";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else {
        if (count == 42) {
          if (width == 7) {
            if (height == 10) {
              return "D";
            } else if (height == 13) {
              return "Ä";
            } else /* NOLINT */ {}
          } else if (width == 8) {
            return "K";
          } else if (width == 10) {
            return "vi";
          } else /* NOLINT */ {}
        } else if (count == 43) {
          if (width == 8) {
            return "R";
          } else if (width == 10) {
            return "m";
          } else /* NOLINT */ {}
        } else if (count == 44) {
          if (width == 8) {
            if (height == 10) {
              if (pixel(0) == 1) {
                return "H";
              } else if (pixel(0) == 0) {
                return "&";
              } else /* NOLINT */ {}
            } else if (height == 13) {
              if (pixel(24) == 1) {
                return "Ù";
              } else if (pixel(24) == 0) {
                return "Ò";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 9) {
            return "Q";
          } else if (width == 10) {
            return "xi";
          } else /* NOLINT */ {}
        } else if (count == 45) {
          return "xt";
        } else /* NOLINT */ {}
      }
    } else {
      if (count < 50) {
        if (count == 46) {
          if (width == 7) {
            return "B";
          } else if (width == 8) {
            if (height == 10) {
              return "N";
            } else if (height == 13) {
              return "Õ";
            } else /* NOLINT */ {}
          } else if (width == 12) {
            return "xs";
          } else /* NOLINT */ {}
        } else if (count == 47) {
          if (width == 8) {
            if (height == 13) {
              if (pixel(24) == 1) {
                return "Û";
              } else if (pixel(24) == 0) {
                return "Ô";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 10) {
            return "yi";
          } else if (width == 11) {
            return "fL";
          } else if (width == 13) {
            if (height == 7) {
              if (pixel(7) == 1) {
                return "xz";
              } else if (pixel(7) == 0) {
                return "xc";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 48) {
          if (width == 8) {
            if (height == 13) {
              if (pixel(24) == 1) {
                return "Ü";
              } else if (pixel(24) == 0) {
                return "Ö";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 9) {
            if (height == 10) {
              if (pixel(39) == 1) {
                if (pixel(59) == 1) {
                  return "le";
                } else if (pixel(59) == 0) {
                  return "lo";
                } else /* NOLINT */ {}
              } else if (pixel(39) == 0) {
                return "la";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 12) {
            return "xf";
          } else if (width == 14) {
            return "xv";
          } else /* NOLINT */ {}
        } else if (count == 49) {
          if (width == 12) {
            return "ys";
          } else if (width == 13) {
            return "fT";
          } else if (width == 15) {
            return "_g";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else {
        if (count == 50) {
          return "xx";
        } else if (count == 51) {
          if (width == 9) {
            return "M";
          } else if (width == 13) {
            return "ve";
          } else /* NOLINT */ {}
        } else if (count == 53) {
          if (width == 13) {
            if (height == 7) {
              if (pixel(20) == 1) {
                if (pixel(48) == 1) {
                  return "xe";
                } else if (pixel(48) == 0) {
                  return "xo";
                } else /* NOLINT */ {}
              } else if (pixel(20) == 0) {
                return "xa";
              } else /* NOLINT */ {}
            } else if (height == 10) {
              return "fY";
            } else /* NOLINT */ {}
          } else if (width == 14) {
            return "xy";
          } else /* NOLINT */ {}
        } else if (count == 54) {
          return "W";
        } else /* NOLINT */ {}
      }
    }
  } else {
    if (count < 67) {
      if (count < 60) {
        if (count == 55) {
          return "fS";
        } else if (count == 56) {
          return "fV";
        } else if (count == 58) {
          return "wi";
        } else if (count == 59) {
          if (width == 12) {
            return "fP";
          } else if (width == 13) {
            return "fX";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else {
        if (count == 60) {
          return "xk";
        } else if (count == 63) {
          if (width == 13) {
            return "fU";
          } else if (width == 17) {
            return "xw";
          } else /* NOLINT */ {}
        } else if (count == 65) {
          if (width == 12) {
            return "fD";
          } else if (width == 13) {
            return "fK";
          } else /* NOLINT */ {}
        } else if (count == 66) {
          return "xg";
        } else /* NOLINT */ {}
      }
    } else {
      if (count < 75) {
        if (count == 67) {
          if (width == 13) {
            return "fH";
          } else if (width == 17) {
            return "ow";
          } else /* NOLINT */ {}
        } else if (count == 68) {
          return "xm";
        } else if (count == 69) {
          if (width == 12) {
            return "fB";
          } else if (width == 13) {
            return "fN";
          } else /* NOLINT */ {}
        } else if (count == 71) {
          return "Re";
        } else /* NOLINT */ {}
      } else {
        if (count == 75) {
          return "Ré";
        } else if (count == 77) {
          return "fW";
        } else if (count == 85) {
          return "RD";
        } else if (count == 88) {
          return "ows";
        } else /* NOLINT */ {}
      }
    }
  }
}
