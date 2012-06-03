/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2010, LABUST, UNIZG-FER
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the LABUST nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/
#ifndef XMLFWD_HPP_
#define XMLFWD_HPP_

#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>

struct _xmlDoc;
struct _xmlXPathContext;
struct _xmlXPathObject;
typedef unsigned char xmlChar;
struct _xmlNode;
struct _xmlNodeSet;
struct _xmlTextWriter;
struct _xmlBuffer;

namespace labust
{
  namespace xml
  {
    class Reader;
    class Writer;
    class GyrosReader;
    class GyrosWriter;
    class GyrosMatrix;
    class XMLException;

    typedef boost::shared_ptr<Reader> ReaderPtr;
    typedef boost::shared_ptr<Writer> WriterPtr;

    typedef boost::shared_ptr<_xmlDoc> xmlDocPtr;
    typedef boost::shared_ptr<_xmlXPathContext> xmlXPathContextPtr;
    typedef boost::shared_ptr<_xmlXPathObject> xmlXPathObjectPtr;
    typedef _xmlNode* xmlNodePtr;

    typedef boost::shared_ptr<_xmlTextWriter> xmlTextWriterPtr;
    typedef boost::shared_ptr<_xmlBuffer> xmlBufferPtr;

    typedef boost::shared_ptr<std::vector<xmlNodePtr> > NodeCollectionPtr;

    typedef boost::shared_ptr<std::string> StringPtr;
  }
}
/* XMLFWD_HPP_ */
#endif
