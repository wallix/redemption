if (count < 30) {
  if (count < 16) {
    if (count < 9) {
      if (count < 5) {
        if (count == 2) {
          return ".";
        } else if (count == 3) {
          return "'";
        } else if (count == 4) {
          if (width == 1) {
            if (height == 4) {
              return ",";
            } else if (height == 8) {
              return ":";
            } else {}
          } else if (width == 2) {
            if (height == 2) {
              return ".";
            } else if (height == 3) {
              return "'";
            } else if (height == 4) {
              return ",";
            } else {}
          } else if (width == 4) {
            return "-";
          } else {}
        } else {}
      } else {
        if (count == 5) {
          if (width == 2) {
            return ",";
          } else if (width == 5) {
            return "-";
          } else {}
        } else if (count == 6) {
          if (width == 2) {
            if (height == 3) {
              return "'";
            } else if (height == 10) {
              return ";";
            } else {}
          } else if (width == 6) {
            return "_";
          } else {}
        } else if (count == 8) {
          if (width == 2) {
            return ":";
          } else if (width == 4) {
            return "°";
          } else if (width == 7) {
            return "~";
          } else {}
        } else {}
      }
    } else {
      if (count < 12) {
        if (count == 9) {
          if (width == 1) {
            return "i";
          } else if (width == 7) {
            return "~";
          } else {}
        } else if (count == 10) {
          if (width == 1) {
            return "!";
          } else if (width == 2) {
            return "i";
          } else if (width == 4) {
            return "°";
          } else if (width == 8) {
            return "~";
          } else {}
        } else if (count == 11) {
          if (width == 1) {
            return "l";
          } else if (width == 4) {
            return "r";
          } else {}
        } else {}
      } else {
        if (count == 12) {
          if (width == 4) {
            if (height == 8) {
              return "r";
            } else if (height == 11) {
              return "î";
            } else {}
          } else if (width == 5) {
            return "*";
          } else if (width == 6) {
            if (height == 7) {
              if (pixel(0) == 1) {
                return ">";
              } else if (pixel(0) == 0) {
                return "<";
              } else {}
            } else {}
          } else {}
        } else if (count == 13) {
          if (width == 3) {
            if (height == 11) {
              return "1";
            } else if (height == 13) {
              if (pixel(3) == 1) {
                return ")";
              } else if (pixel(3) == 0) {
                if (pixel(0) == 1) {
                  return "}";
                } else if (pixel(0) == 0) {
                  if (pixel(4) == 1) {
                    return "{";
                  } else if (pixel(4) == 0) {
                    return "(";
                  } else {}
                } else {}
              } else {}
            } else {}
          } else if (width == 4) {
            if (height == 8) {
              return "r";
            } else if (height == 11) {
              return "J";
            } else {}
          } else if (width == 5) {
            return "c";
          } else if (width == 7) {
            return "+";
          } else {}
        } else if (count == 14) {
          if (width < 5) {
            if (width == 1) {
              return "|";
            } else if (width == 3) {
              if (height == 13) {
                if (pixel(0) == 1) {
                  return ")";
                } else if (pixel(0) == 0) {
                  return "(";
                } else {}
              } else {}
            } else if (width == 4) {
              if (height == 8) {
                return "s";
              } else if (height == 10) {
                return "t";
              } else if (height == 11) {
                return "J";
              } else {}
            } else {}
          } else {
            if (width == 5) {
              return "?";
            } else if (width == 6) {
              if (height == 13) {
                if (pixel(0) == 1) {
                  return "/";
                } else if (pixel(0) == 0) {
                  return "\\";
                } else {}
              } else {}
            } else if (width == 7) {
              return "=";
            } else {}
          }
        } else if (count == 15) {
          if (width == 3) {
            if (height == 13) {
              if (pixel(1) == 1) {
                return "{";
              } else if (pixel(1) == 0) {
                if (pixel(0) == 1) {
                  return ")";
                } else if (pixel(0) == 0) {
                  return "(";
                } else {}
              } else {}
            } else {}
          } else if (width == 4) {
            if (height == 10) {
              return "t";
            } else if (height == 11) {
              if (pixel(2) == 1) {
                return "f";
              } else if (pixel(2) == 0) {
                return "1";
              } else {}
            } else if (height == 14) {
              if (pixel(1) == 1) {
                return "Î";
              } else if (pixel(1) == 0) {
                return "j";
              } else {}
            } else {}
          } else if (width == 5) {
            return "L";
          } else if (width == 7) {
            return "v";
          } else {}
        } else {}
      }
    }
  } else {
    if (count < 23) {
      if (count < 19) {
        if (count == 16) {
          if (width == 3) {
            if (height == 13) {
              if (pixel(0) == 1) {
                return ")";
              } else if (pixel(0) == 0) {
                return "(";
              } else {}
            } else {}
          } else if (width == 4) {
            if (height == 11) {
              return "f";
            } else if (height == 14) {
              return "j";
            } else {}
          } else if (width == 5) {
            if (height == 8) {
              if (pixel(4) == 1) {
                return "c";
              } else if (pixel(4) == 0) {
                return "s";
              } else {}
            } else if (height == 10) {
              return "t";
            } else {}
          } else if (width == 6) {
            return "\\";
          } else {}
        } else if (count == 17) {
          if (width < 5) {
            if (width == 3) {
              if (height == 13) {
                if (pixel(3) == 1) {
                  return "[";
                } else if (pixel(3) == 0) {
                  return "]";
                } else {}
              } else {}
            } else if (width == 4) {
              return "{";
            } else {}
          } else {
            if (width == 5) {
              return "f";
            } else if (width == 6) {
              if (height == 8) {
                return "c";
              } else if (height == 11) {
                return "7";
              } else if (height == 13) {
                return "\\";
              } else {}
            } else if (width == 7) {
              return "T";
            } else {}
          }
        } else if (count == 18) {
          if (width < 5) {
            if (width == 2) {
              return "i";
            } else if (width == 4) {
              if (height == 8) {
                return "r";
              } else if (height == 13) {
                if (pixel(9) == 1) {
                  if (pixel(0) == 1) {
                    return "}";
                  } else if (pixel(0) == 0) {
                    return "(";
                  } else {}
                } else if (pixel(9) == 0) {
                  return ")";
                } else {}
              } else {}
            } else {}
          } else {
            if (width == 5) {
              if (height == 8) {
                return "s";
              } else if (height == 12) {
                return "ç";
              } else {}
            } else if (width == 6) {
              return "c";
            } else if (width == 7) {
              return "o";
            } else {}
          }
        } else {}
      } else {
        if (count == 19) {
          if (width == 5) {
            if (height == 8) {
              return "s";
            } else if (height == 11) {
              return "F";
            } else {}
          } else if (width == 6) {
            return "c";
          } else if (width == 7) {
            if (height == 11) {
              if (pixel(1) == 1) {
                return "7";
              } else if (pixel(1) == 0) {
                return "y";
              } else {}
            } else {}
          } else if (width == 8) {
            return "Y";
          } else {}
        } else if (count == 20) {
          if (width < 6) {
            if (width == 4) {
              return "J";
            } else if (width == 5) {
              return "s";
            } else {}
          } else {
            if (width == 6) {
              if (height == 8) {
                if (pixel(11) == 1) {
                  if (pixel(19) == 1) {
                    return "v";
                  } else if (pixel(19) == 0) {
                    if (pixel(1) == 1) {
                      return "x";
                    } else if (pixel(1) == 0) {
                      if (pixel(2) == 1) {
                        return "n";
                      } else if (pixel(2) == 0) {
                        return "u";
                      } else {}
                    } else {}
                  } else {}
                } else if (pixel(11) == 0) {
                  return "z";
                } else {}
              } else {}
            } else if (width == 7) {
              return "ö";
            } else if (width == 8) {
              return "+";
            } else {}
          }
        } else if (count == 21) {
          if (width == 5) {
            return "s";
          } else if (width == 6) {
            if (height == 8) {
              if (pixel(5) == 1) {
                if (pixel(0) == 1) {
                  return "u";
                } else if (pixel(0) == 0) {
                  return "c";
                } else {}
              } else if (pixel(5) == 0) {
                return "n";
              } else {}
            } else {}
          } else if (width == 7) {
            if (height == 8) {
              if (pixel(0) == 1) {
                return "v";
              } else if (pixel(0) == 0) {
                return "z";
              } else {}
            } else if (height == 11) {
              return "7";
            } else if (height == 13) {
              return "ÿ";
            } else {}
          } else {}
        } else if (count == 22) {
          if (width == 2) {
            return "l";
          } else if (width == 4) {
            if (height == 10) {
              return "t";
            } else if (height == 14) {
              return "j";
            } else {}
          } else if (width == 6) {
            if (height == 8) {
              if (pixel(7) == 1) {
                if (pixel(0) == 1) {
                  return "x";
                } else if (pixel(0) == 0) {
                  return "e";
                } else {}
              } else if (pixel(7) == 0) {
                return "u";
              } else {}
            } else if (height == 10) {
              return "ü";
            } else if (height == 11) {
              if (pixel(0) == 1) {
                if (pixel(4) == 1) {
                  return "5";
                } else if (pixel(4) == 0) {
                  return "3";
                } else {}
              } else if (pixel(0) == 0) {
                return "ù";
              } else {}
            } else {}
          } else if (width == 7) {
            if (height == 8) {
              if (pixel(2) == 1) {
                return "z";
              } else if (pixel(2) == 0) {
                if (pixel(1) == 1) {
                  return "x";
                } else if (pixel(1) == 0) {
                  return "v";
                } else {}
              } else {}
            } else if (height == 11) {
              if (pixel(0) == 1) {
                return "Y";
              } else if (pixel(0) == 0) {
                if (pixel(5) == 1) {
                  if (pixel(6) == 1) {
                    return "C";
                  } else if (pixel(6) == 0) {
                    return "£";
                  } else {}
                } else if (pixel(5) == 0) {
                  return "ô";
                } else {}
              } else {}
            } else {}
          } else {}
        } else {}
      }
    } else {
      if (count < 26) {
        if (count == 23) {
          if (width == 4) {
            return "1";
          } else if (width == 5) {
            return "E";
          } else if (width == 6) {
            if (height == 8) {
              return "a";
            } else if (height == 11) {
              if (pixel(0) == 1) {
                return "h";
              } else if (pixel(0) == 0) {
                return "2";
              } else {}
            } else {}
          } else if (width == 7) {
            if (height == 8) {
              if (pixel(7) == 1) {
                if (pixel(0) == 1) {
                  return "v";
                } else if (pixel(0) == 0) {
                  return "o";
                } else {}
              } else if (pixel(7) == 0) {
                return "x";
              } else {}
            } else if (height == 11) {
              return "Y";
            } else {}
          } else {}
        } else if (count == 24) {
          if (width < 6) {
            if (width == 4) {
              if (height == 13) {
                if (pixel(0) == 1) {
                  return "}";
                } else if (pixel(0) == 0) {
                  return "{";
                } else {}
              } else {}
            } else if (width == 5) {
              return "t";
            } else {}
          } else {
            if (width == 6) {
              if (height == 8) {
                return "e";
              } else if (height == 10) {
                return "ë";
              } else if (height == 11) {
                if (pixel(13) == 1) {
                  return "y";
                } else if (pixel(13) == 0) {
                  if (pixel(50) == 1) {
                    return "k";
                  } else if (pixel(50) == 0) {
                    if (pixel(31) == 1) {
                      return "3";
                    } else if (pixel(31) == 0) {
                      if (pixel(28) == 1) {
                        return "h";
                      } else if (pixel(28) == 0) {
                        if (pixel(8) == 1) {
                          return "é";
                        } else if (pixel(8) == 0) {
                          if (pixel(29) == 1) {
                            if (pixel(5) == 1) {
                              return "µ";
                            } else if (pixel(5) == 0) {
                              if (pixel(0) == 1) {
                                return "P";
                              } else if (pixel(0) == 0) {
                                if (pixel(2) == 1) {
                                  if (pixel(4) == 1) {
                                    return "0";
                                  } else if (pixel(4) == 0) {
                                    if (pixel(3) == 1) {
                                      return "û";
                                    } else if (pixel(3) == 0) {
                                      return "ù";
                                    } else {}
                                  } else {}
                                } else if (pixel(2) == 0) {
                                  return "è";
                                } else {}
                              } else {}
                            } else {}
                          } else if (pixel(29) == 0) {
                            return "S";
                          } else {}
                        } else {}
                      } else {}
                    } else {}
                  } else {}
                } else {}
              } else {}
            } else if (width == 7) {
              if (height == 8) {
                return "o";
              } else if (height == 11) {
                if (pixel(0) == 1) {
                  return "y";
                } else if (pixel(0) == 0) {
                  return "2";
                } else {}
              } else {}
            } else if (width == 8) {
              return "U";
            } else {}
          }
        } else if (count == 25) {
          if (width == 5) {
            return "f";
          } else if (width == 6) {
            if (height == 8) {
              if (pixel(6) == 1) {
                return "e";
              } else if (pixel(6) == 0) {
                return "a";
              } else {}
            } else if (height == 10) {
              return "ä";
            } else if (height == 11) {
              if (pixel(5) == 1) {
                return "E";
              } else if (pixel(5) == 0) {
                if (pixel(37) == 1) {
                  return "P";
                } else if (pixel(37) == 0) {
                  if (pixel(0) == 1) {
                    return "h";
                  } else if (pixel(0) == 0) {
                    if (pixel(3) == 1) {
                      if (pixel(1) == 1) {
                        return "2";
                      } else if (pixel(1) == 0) {
                        return "6";
                      } else {}
                    } else if (pixel(3) == 0) {
                      return "à";
                    } else {}
                  } else {}
                } else {}
              } else {}
            } else if (height == 12) {
              return "ç";
            } else {}
          } else if (width == 7) {
            if (height == 8) {
              return "o";
            } else if (height == 11) {
              if (pixel(0) == 1) {
                return "y";
              } else if (pixel(0) == 0) {
                return "q";
              } else {}
            } else {}
          } else if (width == 8) {
            if (height == 11) {
              if (pixel(3) == 1) {
                return "C";
              } else if (pixel(3) == 0) {
                return "4";
              } else {}
            } else {}
          } else {}
        } else {}
      } else {
        if (count == 26) {
          if (width < 8) {
            if (width == 5) {
              return "§";
            } else if (width == 6) {
              if (height == 8) {
                return "a";
              } else if (height == 11) {
                if (pixel(2) == 1) {
                  if (pixel(3) == 1) {
                    if (pixel(32) == 1) {
                      return "S";
                    } else if (pixel(32) == 0) {
                      if (pixel(1) == 1) {
                        if (pixel(24) == 1) {
                          if (pixel(7) == 1) {
                            if (pixel(10) == 1) {
                              return "9";
                            } else if (pixel(10) == 0) {
                              return "5";
                            } else {}
                          } else if (pixel(7) == 0) {
                            return "P";
                          } else {}
                        } else if (pixel(24) == 0) {
                          return "2";
                        } else {}
                      } else if (pixel(1) == 0) {
                        return "ê";
                      } else {}
                    } else {}
                  } else if (pixel(3) == 0) {
                    return "è";
                  } else {}
                } else if (pixel(2) == 0) {
                  return "L";
                } else {}
              } else {}
            } else if (width == 7) {
              if (height == 8) {
                if (pixel(0) == 1) {
                  return "x";
                } else if (pixel(0) == 0) {
                  if (pixel(1) == 1) {
                    return "o";
                  } else if (pixel(1) == 0) {
                    return "e";
                  } else {}
                } else {}
              } else if (height == 11) {
                if (pixel(2) == 1) {
                  return "p";
                } else if (pixel(2) == 0) {
                  if (pixel(5) == 1) {
                    return "4";
                  } else if (pixel(5) == 0) {
                    if (pixel(0) == 1) {
                      if (pixel(1) == 1) {
                        return "y";
                      } else if (pixel(1) == 0) {
                        return "b";
                      } else {}
                    } else if (pixel(0) == 0) {
                      return "d";
                    } else {}
                  } else {}
                } else {}
              } else {}
            } else {}
          } else {
            if (width == 8) {
              if (height == 8) {
                return "o";
              } else if (height == 11) {
                if (pixel(1) == 1) {
                  return "Y";
                } else if (pixel(1) == 0) {
                  return "U";
                } else {}
              } else if (height == 13) {
                return "Ü";
              } else {}
            } else if (width == 9) {
              return "V";
            } else if (width == 11) {
              return "rv";
            } else {}
          }
        } else if (count == 27) {
          if (width == 6) {
            if (height == 11) {
              if (pixel(58) == 1) {
                if (pixel(0) == 1) {
                  return "k";
                } else if (pixel(0) == 0) {
                  if (pixel(3) == 1) {
                    if (pixel(1) == 1) {
                      return "3";
                    } else if (pixel(1) == 0) {
                      return "â";
                    } else {}
                  } else if (pixel(3) == 0) {
                    return "à";
                  } else {}
                } else {}
              } else if (pixel(58) == 0) {
                return "è";
              } else {}
            } else if (height == 12) {
              return "ç";
            } else if (height == 13) {
              return "Ë";
            } else if (height == 14) {
              if (pixel(2) == 1) {
                return "È";
              } else if (pixel(2) == 0) {
                return "É";
              } else {}
            } else {}
          } else if (width == 7) {
            if (height == 8) {
              return "e";
            } else if (height == 11) {
              if (pixel(0) == 1) {
                return "U";
              } else if (pixel(0) == 0) {
                if (pixel(2) == 1) {
                  return "S";
                } else if (pixel(2) == 0) {
                  return "4";
                } else {}
              } else {}
            } else {}
          } else if (width == 8) {
            if (height == 11) {
              if (pixel(11) == 1) {
                return "C";
              } else if (pixel(11) == 0) {
                return "G";
              } else {}
            } else {}
          } else {}
        } else if (count == 28) {
          if (width == 6) {
            if (height == 11) {
              if (pixel(2) == 1) {
                if (pixel(23) == 1) {
                  return "à";
                } else if (pixel(23) == 0) {
                  if (pixel(1) == 1) {
                    return "S";
                  } else if (pixel(1) == 0) {
                    return "è";
                  } else {}
                } else {}
              } else if (pixel(2) == 0) {
                return "é";
              } else {}
            } else {}
          } else if (width == 7) {
            return "P";
          } else if (width == 8) {
            if (height == 11) {
              if (pixel(9) == 1) {
                return "Y";
              } else if (pixel(9) == 0) {
                if (pixel(1) == 1) {
                  return "T";
                } else if (pixel(1) == 0) {
                  if (pixel(6) == 1) {
                    return "K";
                  } else if (pixel(6) == 0) {
                    return "H";
                  } else {}
                } else {}
              } else {}
            } else if (height == 14) {
              return "Û";
            } else {}
          } else {}
        } else if (count == 29) {
          if (width == 6) {
            if (height == 8) {
              return "a";
            } else if (height == 11) {
              if (pixel(0) == 1) {
                return "F";
              } else if (pixel(0) == 0) {
                return "à";
              } else {}
            } else if (height == 14) {
              return "Ê";
            } else {}
          } else if (width == 7) {
            if (height == 11) {
              if (pixel(1) == 1) {
                return "Z";
              } else if (pixel(1) == 0) {
                if (pixel(19) == 1) {
                  return "p";
                } else if (pixel(19) == 0) {
                  if (pixel(22) == 1) {
                    return "S";
                  } else if (pixel(22) == 0) {
                    if (pixel(0) == 1) {
                      return "b";
                    } else if (pixel(0) == 0) {
                      if (pixel(34) == 1) {
                        if (pixel(4) == 1) {
                          if (pixel(2) == 1) {
                            if (pixel(5) == 1) {
                              return "q";
                            } else if (pixel(5) == 0) {
                              return "g";
                            } else {}
                          } else if (pixel(2) == 0) {
                            return "é";
                          } else {}
                        } else if (pixel(4) == 0) {
                          return "d";
                        } else {}
                      } else if (pixel(34) == 0) {
                        return "è";
                      } else {}
                    } else {}
                  } else {}
                } else {}
              } else {}
            } else {}
          } else if (width == 8) {
            if (height == 11) {
              if (pixel(1) == 1) {
                return "Z";
              } else if (pixel(1) == 0) {
                return "C";
              } else {}
            } else {}
          } else {}
        } else {}
      }
    }
  }
} else {
  if (count < 43) {
    if (count < 36) {
      if (count < 33) {
        if (count == 30) {
          if (width == 6) {
            if (height == 8) {
              return "a";
            } else if (height == 11) {
              return "à";
            } else {}
          } else if (width == 7) {
            if (height == 11) {
              if (pixel(47) == 1) {
                if (pixel(6) == 1) {
                  return "q";
                } else if (pixel(6) == 0) {
                  if (pixel(5) == 1) {
                    return "é";
                  } else if (pixel(5) == 0) {
                    if (pixel(0) == 1) {
                      if (pixel(2) == 1) {
                        return "p";
                      } else if (pixel(2) == 0) {
                        return "b";
                      } else {}
                    } else if (pixel(0) == 0) {
                      return "è";
                    } else {}
                  } else {}
                } else {}
              } else if (pixel(47) == 0) {
                return "d";
              } else {}
            } else {}
          } else if (width == 8) {
            if (height == 8) {
              return "o";
            } else if (height == 11) {
              if (pixel(0) == 1) {
                return "V";
              } else if (pixel(0) == 0) {
                return "G";
              } else {}
            } else {}
          } else if (width == 9) {
            if (height == 11) {
              if (pixel(1) == 1) {
                return "Z";
              } else if (pixel(1) == 0) {
                return "rf";
              } else {}
            } else {}
          } else {}
        } else if (count == 31) {
          if (width == 6) {
            return "6";
          } else if (width == 7) {
            if (height == 11) {
              if (pixel(35) == 1) {
                if (pixel(51) == 1) {
                  return "p";
                } else if (pixel(51) == 0) {
                  if (pixel(2) == 1) {
                    return "0";
                  } else if (pixel(2) == 0) {
                    if (pixel(0) == 1) {
                      return "K";
                    } else if (pixel(0) == 0) {
                      if (pixel(3) == 1) {
                        return "6";
                      } else if (pixel(3) == 0) {
                        return "d";
                      } else {}
                    } else {}
                  } else {}
                } else {}
              } else if (pixel(35) == 0) {
                return "9";
              } else {}
            } else {}
          } else if (width == 8) {
            if (height == 11) {
              if (pixel(0) == 1) {
                if (pixel(1) == 1) {
                  return "Z";
                } else if (pixel(1) == 0) {
                  return "V";
                } else {}
              } else if (pixel(0) == 0) {
                return "G";
              } else {}
            } else {}
          } else if (width == 9) {
            if (height == 11) {
              if (pixel(2) == 1) {
                return "D";
              } else if (pixel(2) == 0) {
                return "V";
              } else {}
            } else {}
          } else {}
        } else if (count == 32) {
          if (width == 7) {
            if (height == 11) {
              if (pixel(1) == 1) {
                return "R";
              } else if (pixel(1) == 0) {
                if (pixel(2) == 1) {
                  return "g";
                } else if (pixel(2) == 0) {
                  if (pixel(0) == 1) {
                    return "K";
                  } else if (pixel(0) == 0) {
                    return "6";
                  } else {}
                } else {}
              } else {}
            } else {}
          } else if (width == 8) {
            if (height == 11) {
              if (pixel(2) == 1) {
                return "R";
              } else if (pixel(2) == 0) {
                return "X";
              } else {}
            } else {}
          } else if (width == 9) {
            if (height == 11) {
              if (pixel(0) == 1) {
                return "V";
              } else if (pixel(0) == 0) {
                return "O";
              } else {}
            } else {}
          } else if (width == 11) {
            return "m";
          } else {}
        } else {}
      } else {
        if (count == 33) {
          if (width < 9) {
            if (width == 6) {
              if (height == 11) {
                if (pixel(0) == 1) {
                  if (pixel(5) == 1) {
                    return "E";
                  } else if (pixel(5) == 0) {
                    return "B";
                  } else {}
                } else if (pixel(0) == 0) {
                  return "à";
                } else {}
              } else {}
            } else if (width == 7) {
              if (height == 8) {
                if (pixel(2) == 1) {
                  return "n";
                } else if (pixel(2) == 0) {
                  return "u";
                } else {}
              } else if (height == 11) {
                return "4";
              } else {}
            } else if (width == 8) {
              if (height == 11) {
                if (pixel(0) == 1) {
                  if (pixel(2) == 1) {
                    return "D";
                  } else if (pixel(2) == 0) {
                    return "X";
                  } else {}
                } else if (pixel(0) == 0) {
                  return "G";
                } else {}
              } else {}
            } else {}
          } else {
            if (width == 9) {
              if (height == 11) {
                if (pixel(1) == 1) {
                  return "X";
                } else if (pixel(1) == 0) {
                  return "A";
                } else {}
              } else {}
            } else if (width == 10) {
              return "A";
            } else if (width == 11) {
              if (height == 8) {
                return "rv";
              } else if (height == 10) {
                return "ct";
              } else {}
            } else {}
          }
        } else if (count == 34) {
          if (width == 7) {
            if (height == 8) {
              return "n";
            } else if (height == 11) {
              return "8";
            } else {}
          } else if (width == 8) {
            if (height == 11) {
              if (pixel(15) == 1) {
                return "U";
              } else if (pixel(15) == 0) {
                if (pixel(2) == 1) {
                  if (pixel(5) == 1) {
                    return "D";
                  } else if (pixel(5) == 0) {
                    return "R";
                  } else {}
                } else if (pixel(2) == 0) {
                  return "X";
                } else {}
              } else {}
            } else {}
          } else if (width == 9) {
            if (height == 11) {
              return "A";
            } else if (height == 13) {
              return "Ö";
            } else {}
          } else if (width == 11) {
            if (height == 8) {
              return "w";
            } else if (height == 10) {
              return "ct";
            } else {}
          } else {}
        } else if (count == 35) {
          if (width < 8) {
            if (width == 6) {
              if (height == 11) {
                if (pixel(7) == 1) {
                  if (pixel(0) == 1) {
                    return "k";
                  } else if (pixel(0) == 0) {
                    return "8";
                  } else {}
                } else if (pixel(7) == 0) {
                  return "B";
                } else {}
              } else {}
            } else if (width == 7) {
              if (height == 11) {
                if (pixel(8) == 1) {
                  if (pixel(13) == 1) {
                    return "g";
                  } else if (pixel(13) == 0) {
                    if (pixel(0) == 1) {
                      return "k";
                    } else if (pixel(0) == 0) {
                      if (pixel(6) == 1) {
                        return "fi";
                      } else if (pixel(6) == 0) {
                        return "8";
                      } else {}
                    } else {}
                  } else {}
                } else if (pixel(8) == 0) {
                  return "B";
                } else {}
              } else {}
            } else {}
          } else {
            if (width == 8) {
              if (height == 11) {
                if (pixel(8) == 1) {
                  return "U";
                } else if (pixel(8) == 0) {
                  return "X";
                } else {}
              } else {}
            } else if (width == 9) {
              return "G";
            } else if (width == 10) {
              if (height == 11) {
                return "%";
              } else if (height == 13) {
                return "Ä";
              } else if (height == 14) {
                if (pixel(4) == 1) {
                  return "À";
                } else if (pixel(4) == 0) {
                  return "Á";
                } else {}
              } else {}
            } else {}
          }
        } else {}
      }
    } else {
      if (count < 39) {
        if (count == 36) {
          if (width < 8) {
            if (width == 6) {
              if (height == 11) {
                return "B";
              } else if (height == 14) {
                return "$";
              } else {}
            } else if (width == 7) {
              if (height == 11) {
                if (pixel(4) == 1) {
                  return "g";
                } else if (pixel(4) == 0) {
                  return "ù";
                } else {}
              } else {}
            } else {}
          } else {
            if (width == 8) {
              return "N";
            } else if (width == 9) {
              if (height == 11) {
                return "D";
              } else if (height == 14) {
                return "Ô";
              } else {}
            } else if (width == 10) {
              return "w";
            } else {}
          }
        } else if (count == 37) {
          if (width == 7) {
            return "P";
          } else if (width == 10) {
            if (height == 8) {
              if (pixel(2) == 1) {
                return "m";
              } else if (pixel(2) == 0) {
                return "w";
              } else {}
            } else if (height == 11) {
              return "O";
            } else if (height == 14) {
              return "Â";
            } else {}
          } else {}
        } else if (count == 38) {
          if (width == 7) {
            if (height == 11) {
              if (pixel(1) == 1) {
                if (pixel(2) == 1) {
                  if (pixel(0) == 1) {
                    return "p";
                  } else if (pixel(0) == 0) {
                    return "q";
                  } else {}
                } else if (pixel(2) == 0) {
                  return "b";
                } else {}
              } else if (pixel(1) == 0) {
                return "d";
              } else {}
            } else {}
          } else if (width == 8) {
            if (height == 11) {
              if (pixel(18) == 1) {
                return "N";
              } else if (pixel(18) == 0) {
                return "H";
              } else {}
            } else {}
          } else if (width == 10) {
            if (height == 8) {
              return "w";
            } else if (height == 11) {
              return "O";
            } else {}
          } else if (width == 11) {
            if (height == 8) {
              if (pixel(1) == 1) {
                return "w";
              } else if (pixel(1) == 0) {
                return "m";
              } else {}
            } else if (height == 11) {
              return "ry";
            } else {}
          } else {}
        } else {}
      } else {
        if (count == 39) {
          if (width < 9) {
            if (width == 7) {
              if (height == 11) {
                if (pixel(34) == 1) {
                  if (pixel(0) == 1) {
                    return "b";
                  } else if (pixel(0) == 0) {
                    return "d";
                  } else {}
                } else if (pixel(34) == 0) {
                  return "h";
                } else {}
              } else {}
            } else if (width == 8) {
              return "N";
            } else {}
          } else {
            if (width == 9) {
              return "G";
            } else if (width == 10) {
              if (height == 11) {
                return "O";
              } else if (height == 12) {
                return "Q";
              } else {}
            } else if (width == 11) {
              if (height == 8) {
                if (pixel(5) == 1) {
                  return "rv";
                } else if (pixel(5) == 0) {
                  return "m";
                } else {}
              } else {}
            } else {}
          }
        } else if (count == 40) {
          if (width == 7) {
            return "g";
          } else if (width == 9) {
            return "G";
          } else if (width == 10) {
            if (height == 11) {
              return "&";
            } else if (height == 12) {
              return "Q";
            } else {}
          } else if (width == 11) {
            return "&";
          } else {}
        } else if (count == 41) {
          if (width == 8) {
            return "K";
          } else if (width == 11) {
            if (height == 8) {
              return "m";
            } else if (height == 11) {
              return "%";
            } else if (height == 12) {
              return "Q";
            } else {}
          } else if (width == 12) {
            return "rv";
          } else {}
        } else if (count == 42) {
          if (width == 7) {
            return "g";
          } else if (width == 10) {
            return "O";
          } else if (width == 11) {
            return "Q";
          } else {}
        } else {}
      }
    }
  } else {
    if (count < 53) {
      if (count < 46) {
        if (count == 43) {
          if (width == 7) {
            return "fi";
          } else if (width == 8) {
            return "R";
          } else if (width == 11) {
            if (height == 11) {
              if (pixel(4) == 1) {
                return "ry";
              } else if (pixel(4) == 0) {
                return "%";
              } else {}
            } else if (height == 12) {
              return "Q";
            } else {}
          } else if (width == 12) {
            return "%";
          } else {}
        } else if (count == 44) {
          if (width == 8) {
            return "R";
          } else if (width == 11) {
            if (height == 11) {
              return "&";
            } else if (height == 12) {
              return "Q";
            } else {}
          } else if (width == 12) {
            return "ry";
          } else {}
        } else if (count == 45) {
          if (width == 7) {
            return "B";
          } else if (width == 10) {
            return "M";
          } else {}
        } else {}
      } else {
        if (count == 46) {
          return "D";
        } else if (count == 48) {
          return "N";
        } else if (count == 49) {
          if (width == 9) {
            return "N";
          } else if (width == 10) {
            return "M";
          } else if (width == 11) {
            return "&";
          } else {}
        } else if (count == 51) {
          if (width == 11) {
            return "&";
          } else if (width == 12) {
            return "@";
          } else {}
        } else {}
      }
    } else {
      if (count < 57) {
        if (count == 53) {
          return "W";
        } else if (count == 54) {
          if (width == 12) {
            return "@";
          } else if (width == 15) {
            return "tw";
          } else {}
        } else if (count == 56) {
          if (width == 9) {
            return "N";
          } else if (width == 12) {
            return "@";
          } else {}
        } else {}
      } else {
        if (count == 57) {
          return "W";
        } else if (count == 58) {
          return "W";
        } else if (count == 59) {
          if (width == 11) {
            return "M";
          } else if (width == 12) {
            return "@";
          } else if (width == 13) {
            return "W";
          } else if (width == 14) {
            return "W";
          } else {}
        } else if (count == 60) {
          if (width == 11) {
            return "M";
          } else if (width == 12) {
            return "@";
          } else {}
        } else {}
      }
    }
  }
}
