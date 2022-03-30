/* gnu.classpath.tools.doclets.xmldoclet.Driver1_4
   Copyright (C) 2001 Free Software Foundation, Inc.

This file is part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA.

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version. */

package gnu.classpath.tools.doclets.xmldoclet;

import com.sun.javadoc.*;
import java.io.IOException;

/**
 *  A Doclet which retrieves all information presented by the Doclet
 *  API, dumping it to stdout in XML format.
 *
 *  Supports Doclet API Version 1.4.
 *
 *  @author Julian Scheid
 */
public class Driver1_4 extends Driver {

   /**
    *  Official Doclet entry point.
    */
   public static boolean start(RootDoc root) {

      // Create a new XmlDoclet instance and delegate control.
      return new Driver1_4().instanceStart(root);
   }

   /* since 1.4
   private void outputSourcePosition(int level, SourcePosition sourcePosition) {
      println(level, "<sourceposition"
              + " file=\""+sourcePosition.file().toString()+"\""
              + " line=\""+sourcePosition.line()+"\""
              + " column=\""+sourceposition.column()+"\""
              + "/>");
   }
   */

   protected void outputClassDoc(ClassDoc classDoc)
      throws IOException
   {
      super.outputClassDoc(classDoc);
      //outputSourcePosition(level, doc.position());
   }

   protected void outputFieldDocBody(int level, FieldDoc fieldDoc) {
      super.outputFieldDocBody(level, fieldDoc);
      //println(level, "<constantValueExpression>"+fieldDoc.constantValueExpression()+"</constantValueExpression>");
   }

}
