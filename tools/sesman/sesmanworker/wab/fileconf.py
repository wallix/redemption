from logger import Logger

def parse_conf_file(conf, path):
    from ConfigParser import ConfigParser

    parser = ConfigParser()
    parser.read(path)

    for section in conf:
        for key in conf[section]:
            try:
                if parser.get(section, key):
                    temp = parser.get(section, key)
                    if type(temp) is unicode:
                       conf[section][key] = temp
                    else:
                        try:
                            conf[section][key] = temp.decode("utf-8")
                        except Exception, e:
                            Logger().warning(u"Failed to convert string at (%s, %s) to unicode:" % (section, key))
            except Exception, e:
                Logger().warning(u"Failed to access to key in section (%s, %s)" % (section, key))

