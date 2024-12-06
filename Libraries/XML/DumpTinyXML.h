#pragma once

#include "tinyxml.h"

// ----------------------------------------------------------------------
// STDOUT dump and indenting utility functions
// ----------------------------------------------------------------------
const unsigned int NUM_INDENTS_PER_SPACE = 2;

const char *getIndent(unsigned int numIndents);

// same as getIndent but no "+" at the end
const char *getIndentAlt(unsigned int numIndents);

int dump_attribs_to_stdout(TiXmlElement *pElement, unsigned int indent);

void dump_to_stdout(TiXmlNode *pParent, unsigned int indent = 0);

// load the named file and dump its structure to STDOUT
void dump_to_stdout(const char *pFilename);
