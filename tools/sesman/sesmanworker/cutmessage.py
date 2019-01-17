#!/usr/bin/python -O
# -*- coding: utf-8 -*-

##
# Copyright (c) 2011 WALLIX, SARL. All rights reserved.
# Licensed computer software. Property of WALLIX.
# Product name: WALLIX Admin Bastion V 2.x
# Author(s): Christophe Grosjean
# Id: $Id$
# URL: $URL$
# Module description:  cutmessage utility for Sesman Worker
##
import unittest

def cut_message(message, width = 75, in_cr = '\n', out_cr = '<br>', margin = 6):
    result = []
    for line in message.strip().split(in_cr):
        while len(line) > width:
            end = line[width:].split(' ')

            if len(end[0]) <= margin:
                result.append((line[:width] + end[0]).rstrip())
                end = end[1:]
            else:
                result.append(line[:width] + end[0][:margin] + '-')
                end[0] = '-' + end[0][margin:]

            line = ' '.join(end)

        result.append(line.rstrip())

    return out_cr.join(result)


class TestMotd(unittest.TestCase):
    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_cutLittleMessage(self):
        self.assertEquals("My text", cut_message("My text"))

    def test_cutMessageReplaceCR(self):
        message = cut_message(
            "This is\na very\nlong sentence.\n", 100, '\n', '<br>')
        self.assertEquals(
            "This is<br>a very<br>long sentence.<br>",
            message)

    def test_cut_message_replace_cr_remove_useless_spaces_at_end(self):
        message = cut_message(
            " This is \na very \nlong sentence.\n", 100, '\n', '<br>')
        self.assertEquals(
            " This is<br>a very<br>long sentence.<br>",
            message)

    def test_cut_message_split_at_10(self):
        message = cut_message(
            " This is a very  long sentence.", 10, '\n', '<br>')
        self.assertEquals(
            " This is a<br>very  long<br>sentence.",
            message)

    def test_cutTwoLine(self):
        message = cut_message(
            "My first sentence is long.\n"
            "My second one is long also.", 20,'\n', '<br>')
        self.assertEquals(
            'My first sentence is<br>'
            'long.<br>'
            'My second one is long<br>'
            'also.', message)

    def test_with_cut_after_word(self):
        message = cut_message(
            "This sentence contains a too long word.", 20, '\n', '<br>')
        self.assertEquals(
            'This sentence contains<br>'
            'a too long word.', message)

    def test_cut_too_long_word(self):
        message = cut_message(
            "This sentence contains a looooooooooooooonnnnnnnngggggg word.",
                30, '\n', '<br>', 5)
        self.assertEquals(
            "This sentence contains a looooooooo-<br>"
            "-oooooonnnnnnnngggggg word.", message)

    def test_with_motd_en(self):
        message = cut_message(
          "Welcome on Wallix AdminBastion\n"
          "--\n"
          "Your actions could be recorded and stored in electronic format.\n"
          "Please contact your AdminBastion administrator for more precisions.\n")
        self.assertEquals(
            "Welcome on Wallix AdminBastion<br>"
            "--<br>"
            "Your actions could be recorded and stored in electronic format.<br>"
            "Please contact your AdminBastion administrator for more precisions.<br>",
            message)

    def test_with_motd_fr(self):
        message = cut_message(
            u"Bienvenue sur Wallix AdminBastion\n"
            u"--\n"
            u"Vos actions sont susceptibles d'être enregistrées et conservées sous format électronique.\n"
            u"Merci de contacter l'administrateur AdminBastion pour plus d'informations.\n", 75)
        self.assertEquals(
            u"Bienvenue sur Wallix AdminBastion<br>"
            u"--<br>"
            u"Vos actions sont susceptibles d'être enregistrées et conservées sous format<br>"
            u"électronique.<br>"
            u"Merci de contacter l'administrateur AdminBastion pour plus d'informations.<br>",
            message)
