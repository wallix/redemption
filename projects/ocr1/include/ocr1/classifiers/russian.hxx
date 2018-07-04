if (count < 19) {
  if (count < 10) {
    if (count < 6) {
      if (count == 2) {
        return ".";
      } else if (count == 3) {
        if (width == 1) {
          return "'";
        } else if (width == 2) {
          return ",";
        } else if (width == 3) {
          return "-";
        } else {}
      } else if (count == 4) {
        return ":";
      } else if (count == 5) {
        if (width == 2) {
          return ";";
        } else if (width == 5) {
          return "_";
        } else {}
      } else {}
    } else /* NOLINT */ {
      if (count == 6) {
        if (width == 1) {
          return "\x1";
        } else if (width == 6) {
          return "~";
        } else {}
      } else if (count == 7) {
        return "i";
      } else if (count == 8) {
        return "!";
      } else if (count == 9) {
        if (width == 1) {
          return "\x2";
        } else if (width == 4) {
          return "г";
        } else if (width == 5) {
          return "*";
        } else {}
      } else {}
    }
  } else /* NOLINT */ {
    if (count < 14) {
      if (count == 10) {
        if (width == 4) {
          return "c";
        } else if (width == 5) {
          if (height == 6) {
            if (pixel(1) == 1) {
              return "т";
            } else if (pixel(1) == 0) {
              return "х";
            } else {}
          } else {}
        } else {}
      } else if (count == 11) {
        if (width == 3) {
          if (height == 11) {
            if (pixel(0) == 1) {
              return ")";
            } else if (pixel(0) == 0) {
              return "(";
            } else {}
          } else {}
        } else if (width == 4) {
          return "з";
        } else {}
      } else if (count == 12) {
        if (width == 4) {
          if (height == 6) {
            if (pixel(1) == 1) {
              return "э";
            } else if (pixel(1) == 0) {
              return "к";
            } else {}
          } else if (height == 9) {
            return "7";
          } else {}
        } else if (width == 5) {
          return "ч";
        } else if (width == 12) {
          return "—";
        } else {}
      } else if (count == 13) {
        if (width < 4) {
          if (width == 2) {
            if (height == 11) {
              if (pixel(2) == 1) {
                return "[";
              } else if (pixel(2) == 0) {
                return "]";
              } else {}
            } else {}
          } else if (width == 3) {
            return "I";
          } else {}
        } else /* NOLINT */ {
          if (width == 4) {
            return "e";
          } else if (width == 5) {
            return "Г";
          } else if (width == 7) {
            return "ѵ";
          } else {}
        }
      } else {}
    } else /* NOLINT */ {
      if (count < 16) {
        if (count == 14) {
          if (width == 4) {
            if (height == 6) {
              if (pixel(0) == 1) {
                return "a";
              } else if (pixel(0) == 0) {
                return "я";
              } else {}
            } else if (height == 9) {
              if (pixel(17) == 1) {
                return "3";
              } else if (pixel(17) == 0) {
                return "2";
              } else {}
            } else {}
          } else if (width == 5) {
            if (height == 6) {
              if (pixel(0) == 1) {
                return "ь";
              } else if (pixel(0) == 0) {
                return "o";
              } else {}
            } else {}
          } else if (width == 6) {
            return "л";
          } else {}
        } else if (count == 15) {
          if (width == 4) {
            if (height == 6) {
              return "в";
            } else if (height == 8) {
              return "ё";
            } else {}
          } else if (width == 5) {
            if (height == 6) {
              if (pixel(1) == 1) {
                return "п";
              } else if (pixel(1) == 0) {
                return "н";
              } else {}
            } else if (height == 9) {
              return "1";
            } else {}
          } else if (width == 6) {
            return "C";
          } else if (width == 7) {
            if (height == 9) {
              if (pixel(1) == 1) {
                return "T";
              } else if (pixel(1) == 0) {
                return "У";
              } else {}
            } else {}
          } else {}
        } else {}
      } else /* NOLINT */ {
        if (count == 16) {
          if (width == 4) {
            return "5";
          } else if (width == 5) {
            return "и";
          } else if (width == 6) {
            return "у";
          } else if (width == 7) {
            return "ъ";
          } else {}
        } else if (count == 17) {
          if (width == 4) {
            return "6";
          } else if (width == 5) {
            return "З";
          } else if (width == 7) {
            return "X";
          } else {}
        } else if (count == 18) {
          if (width == 4) {
            if (height == 9) {
              if (pixel(0) == 1) {
                return "E";
              } else if (pixel(0) == 0) {
                if (pixel(16) == 1) {
                  if (pixel(20) == 1) {
                    return "0";
                  } else if (pixel(20) == 0) {
                    return "9";
                  } else {}
                } else if (pixel(16) == 0) {
                  return "8";
                } else {}
              } else {}
            } else {}
          } else if (width == 5) {
            if (height == 9) {
              if (pixel(0) == 1) {
                return "Ь";
              } else if (pixel(0) == 0) {
                return "4";
              } else {}
            } else {}
          } else if (width == 6) {
            if (height == 8) {
              return "ц";
            } else if (height == 9) {
              if (pixel(10) == 1) {
                return "K";
              } else if (pixel(10) == 0) {
                return "Ч";
              } else {}
            } else {}
          } else if (width == 7) {
            return "ж";
          } else {}
        } else {}
      }
    }
  }
} else /* NOLINT */ {
  if (count < 27) {
    if (count < 23) {
      if (count == 19) {
        if (width == 5) {
          if (height == 9) {
            if (pixel(0) == 1) {
              if (pixel(1) == 1) {
                return "P";
              } else if (pixel(1) == 0) {
                return "p";
              } else {}
            } else if (pixel(0) == 0) {
              return "б";
            } else {}
          } else {}
        } else if (width == 6) {
          return "Э";
        } else {}
      } else if (count == 20) {
        if (width < 6) {
          if (width == 4) {
            return "Ё";
          } else if (width == 5) {
            return "й";
          } else {}
        } else /* NOLINT */ {
          if (width == 6) {
            return "ѳ";
          } else if (width == 7) {
            return "O";
          } else if (width == 9) {
            return "Ѵ";
          } else {}
        }
      } else if (count == 21) {
        if (width == 5) {
          return "Б";
        } else if (width == 6) {
          return "д";
        } else if (width == 7) {
          if (height == 9) {
            if (pixel(2) == 1) {
              return "ѣ";
            } else if (pixel(2) == 0) {
              return "Л";
            } else {}
          } else {}
        } else if (width == 8) {
          return "Ъ";
        } else {}
      } else if (count == 22) {
        if (width == 6) {
          return "Я";
        } else if (width == 7) {
          if (height == 6) {
            return "м";
          } else if (height == 9) {
            return "Ѣ";
          } else {}
        } else if (width == 8) {
          if (height == 6) {
            return "ю";
          } else if (height == 9) {
            return "A";
          } else {}
        } else {}
      } else {}
    } else /* NOLINT */ {
      if (count == 23) {
        if (width == 7) {
          if (height == 9) {
            if (pixel(1) == 1) {
              return "П";
            } else if (pixel(1) == 0) {
              return "H";
            } else {}
          } else {}
        } else {}
      } else if (count == 24) {
        if (width == 5) {
          return "В";
        } else if (width == 9) {
          return "ш";
        } else {}
      } else if (count == 25) {
        if (width == 5) {
          return "$";
        } else if (width == 7) {
          if (height == 9) {
            return "Ѳ";
          } else if (height == 11) {
            return "Ц";
          } else {}
        } else {}
      } else if (count == 26) {
        if (width == 7) {
          return "#";
        } else if (width == 10) {
          return "%";
        } else if (width == 11) {
          return "ту";
        } else {}
      } else {}
    }
  } else /* NOLINT */ {
    if (count < 32) {
      if (count == 27) {
        if (width == 7) {
          if (height == 9) {
            if (pixel(0) == 1) {
              return "И";
            } else if (pixel(0) == 0) {
              return "Ф";
            } else {}
          } else {}
        } else if (width == 10) {
          return "щ";
        } else if (width == 11) {
          return "Ж";
        } else {}
      } else if (count == 28) {
        return "ф";
      } else if (count == 30) {
        if (width == 8) {
          return "Д";
        } else if (width == 14) {
          return "ТУ";
        } else {}
      } else if (count == 31) {
        if (width == 7) {
          return "Й";
        } else if (width == 10) {
          return "Ю";
        } else {}
      } else {}
    } else /* NOLINT */ {
      if (count < 36) {
        if (count == 32) {
          return "M";
        } else if (count == 33) {
          if (width == 9) {
            return "Ш";
          } else if (width == 13) {
            return "ѳѵ";
          } else {}
        } else {}
      } else /* NOLINT */ {
        if (count == 36) {
          return "Щ";
        } else if (count == 39) {
          return "КЛ";
        } else if (count == 40) {
          return "@";
        } else {}
      }
    }
  }
}
