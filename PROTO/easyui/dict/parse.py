#!/usr/bin/python
# -*- coding: utf-8 -*-
# Parses "The Project Gutenberg EBook of Webster's Unabridged Dictionary, by Various"
# in plain text format and outputs a .c file that should be compiled with dictionary.c
# genrated by geneet. When executed, this generated program will create an EET file
# with the contents of this dictionary in a structured form, suitable to be queried
# directly with EET.
#
# TODO: Index for partial match search.
#
# Author: Leandro Pereira <leandro@profusion.mobi>

import codecs
import sys
import re
import simplejson

class Pron(object):
  def __init__(self, pron):
    self.pronounciation = pron
    self.definition = ''

  def __str__(self):
    return '<Pronounce as (%s), def as (%s)>' % (self.pronounciation, self.definition)

class Word(object):
  def __init__(self, *words):
    self.word = words[0]
    self.definitions = []
    self.alternate_spelling = set(words[1:])
    self.last_pronounciation = None

  def add_alternate_word(self, other):
    p = Pron(other.word)
    p.definition = 'Also spelled as.'
    self.definitions.append(p)

  def add_pronounciation(self, pron):
    self.last_pronounciation = Pron(pron)
    self.definitions.append(self.last_pronounciation)

  def add_definition(self, definition):
    assert self.last_pronounciation.definition == ''

    self.last_pronounciation.definition = definition

  def __hash__(self):
    return self.word.__hash__()

  def __str__(self):
    defs = '; '.join(str(d) for d in self.definitions)

    return "<Word(%s)%s(defined as %s)>" % (
      self.word.encode('utf-8'),
      ('(alternate_spelling %s)' % '; '.join(self.alternate_spelling)).encode('utf-8') if self.alternate_spelling else '',
      defs
    )

  def __repr__(self):
    return str(self)

words = {}
defbuf = []
last_word = None
state = 'header'
spinner = 0

def add_word(w, lw):
  if not w.word in words:
    words[w.word] = w
    return w
  return lw

path = '%s/29765-8.txt' % (sys.argv[1] if (len(sys.argv) > 1) else '/tmp/dict')

for line in codecs.open(path, encoding='iso-8859-1'):
  if " & " in line: line = line.replace("&", "and")
  line = unicode(line.strip())

  if len(words) % 133 == 0:
    spinner += 1
    sys.stderr.write('\r%s   Processed %d words' % ("/-\\|"[spinner % 4], len(words)))

  if state == 'header':
    if not line:
      continue

    has_alternate_spelling = line[0].isupper() and line[-1].isupper() and ';' in line
    if line.isupper() or has_alternate_spelling:
      if has_alternate_spelling:
        word = Word(*line.split('; '))
      else:
        word = Word(line)

      last_word = add_word(word, last_word)
      state = 'pronounciation'

  elif state == 'pronounciation':
    if line:
      defbuf.append(line)
    else:
      last_word.add_pronounciation(' '.join(defbuf))
      defbuf = []
      state = 'definition'

  elif state == 'definition':
    if not line:
      continue
    if not line.isupper():
      defbuf.append(line)
    else:
      definition = ' '.join(defbuf).strip()
      if definition.startswith('Defn:'):
        definition = definition[len('Defn: '):]
      if definition.startswith(', '):
        definition = definition[len(', '):]

      last_word.add_definition(definition)
      defbuf = []

      last_word = add_word(Word(*line.split('; ')), last_word)
      state = 'pronounciation'

if defbuf:
  last_word.add_definition('\n'.join(defbuf).strip())

sys.stderr.write('\r\033[K')
spinner = 0

print """#include <Eet.h>

int main(int argc, char *argv[])
{
  char *path = (argc > 1) ? argv[1] : "dict.eet";

  eina_init();
  eet_init();

  Eet_File *ef = eet_open(path, EET_FILE_MODE_WRITE);
  if (!ef) return -1;

"""

def filter_for_c(s, link_re = re.compile(r'(\{\\*[\a-zA-Z ]*\})')):
  s = s.replace('\\', '\\\\')
  s = s.replace('"', '\\"')
  s = '\\n'.join(s.split('\n'))
  for r in link_re.findall(s):
    s = s.replace(r, '')
  return s.encode('utf-8')

class RecursiveDict(dict):
   def __missing__(self, key):
      value = self[key] = type(self)()
      return value

t = RecursiveDict()

def add_word_to_tree(word, idx):
  tmp = t
  for char in word:
    tmp = tmp[char]

  tmp["_"] = idx

for index, word in enumerate(words.values()):
  spinner += 1
  if spinner % 133 == 0:
    s = "/-\\|"[spinner % 4]
    p = (100. * spinner) / len(words)
    sys.stderr.write('\r%s   Generating source: %.2f%%' % (s, p))
    print """  fprintf(stderr, "\\r%%c   Generating eet: %.2f%%%%", %d);""" % (p, ord(s))

  for spelling in word.alternate_spelling:
    add_word_to_tree(spelling, index)
  add_word_to_tree(word.word, index)

  for defindex, definition in enumerate(word.definitions):
    print """  eet_write(ef, "%dp%d", "%s", %d, 0);""" % (
      index, defindex, filter_for_c(definition.pronounciation), len(definition.pronounciation)
    )
    print """  eet_write(ef, "%dd%d", "%s", %d, %d);""" % (
      index, defindex, filter_for_c(definition.definition), len(definition.definition),
      len(definition.definition) > 72
    )

def generate_subtree(key, subtree, count = 0):
  if not key in 'ABCDEFGHIJKLMNOPQRSTUVWXYZ': # FIXME Ainda tem algum problema no parser... verificar!
    return

  sys.stderr.write('\r\033[KGenerating index for key %s' % key)
  print """  fprintf(stderr, "\\r   Generating index for key %s");""" % key

  idx_json_out = simplejson.dumps(subtree, separators=',:')
  print """  static const unsigned char json_index_%s[] = {""" % key
  for index, char in enumerate(idx_json_out):
    print '0x%x,' % ord(char),
    if index % 30 == 0:
      print ''
  print """0x00 };"""
  print """  eet_write(ef, "i%s", json_index_%s, %d, 1);""" % (key, key, len(idx_json_out))

for key, subtree in t.items():
  generate_subtree(key, subtree)

print """
  fprintf(stderr, "\\r\\033[KFinished.\\n");
  eet_sync(ef);
  eet_close(ef);
  eet_shutdown();
  eina_shutdown();

  return 0;
}"""

sys.stderr.write('\r\033[KFinished.\n')
