/* DebugDoclet.java - Doclet for debugging
   Copyright (C) 2004 Free Software Foundation, Inc.

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

package gnu.classpath.tools.doclets.debugdoclet;

import com.sun.javadoc.ClassDoc;
import com.sun.javadoc.FieldDoc;
import com.sun.javadoc.MethodDoc;
import com.sun.javadoc.PackageDoc;
import com.sun.javadoc.RootDoc;
import com.sun.javadoc.Tag;

import java.io.PrintStream;

import java.util.Arrays;

public class DebugDoclet
{
   public static boolean start(RootDoc rootDoc)
   {
      new DebugDoclet().run(rootDoc);
      return true;
   }

   private PrintStream out;

   public DebugDoclet()
   {
      out = System.out;
   }

   private void printHeader(String header)
   {
      out.println();
      out.println("******** " + header + " ********");
      out.println();
   }

   private void printSubHeader(String header)
   {
      out.println();
      out.println("======== " + header + " ========");
      out.println();
   }

   private void printSub2Header(String header)
   {
      out.println();
      out.println("-------- " + header + " --------");
      out.println();
   }

   private void run(RootDoc rootDoc)
   {
      printHeader("Overview");

      printSubHeader("Specified Packages");

      PackageDoc[] specifiedPackages = rootDoc.specifiedPackages();
      Arrays.sort(specifiedPackages);
      for (int i=0; i<specifiedPackages.length; ++i) {
         out.println(specifiedPackages[i].name());
      }

      printSubHeader("Specified Classes");

      ClassDoc[] specifiedClasses = rootDoc.specifiedClasses();
      Arrays.sort(specifiedClasses);
      for (int i=0; i<specifiedClasses.length; ++i) {
         out.println(specifiedClasses[i].qualifiedTypeName());
      }
      printSubHeader("Classes");

      ClassDoc[] classes = rootDoc.classes();
      Arrays.sort(classes);
      for (int i=0; i<classes.length; ++i) {
         out.println(classes[i].qualifiedTypeName());
      }

      printHeader("Class Detail");

      for (int i=0; i<classes.length; ++i) {
         printSubHeader(classes[i].qualifiedTypeName());

         printTags(classes[i].firstSentenceTags());

         printSub2Header("Methods");

         MethodDoc[] methods = classes[i].methods();

         for (int j=0; j<methods.length; ++j) {
            out.println("name: \"" + methods[j].name() + "\"");
            out.println("signature: \"" + methods[j].signature() + "\"");
            out.println("modifiers: \"" + methods[j].modifiers() + "\"");
            out.print("throws: ");
            ClassDoc[] thrownExceptions = methods[j].thrownExceptions();
            for (int k=0; k<thrownExceptions.length; ++k) {
               if (k>0) { out.print(", "); }
               out.print(thrownExceptions[k].qualifiedTypeName());
            }
            out.println();
         }

         printSub2Header("Fields");

         FieldDoc[] fields = classes[i].fields();

         for (int j=0; j<fields.length; ++j) {
            out.println("name: \"" + fields[j].name() + "\"");
            out.println("modifiers: \"" + fields[j].modifiers() + "\"");
            out.println();
         }

         printSub2Header("Serializable Fields");

         FieldDoc[] sfields = classes[i].serializableFields();

         for (int j=0; j<sfields.length; ++j) {
            out.println("name: \"" + sfields[j].name() + "\"");
            out.println("modifiers: \"" + sfields[j].modifiers() + "\"");
            out.println();
         }
      }
   }

   private void printTag(Tag tag)
   {
      if (null != tag.text()) {
         System.out.println(tag.text());
      }
   }

   private void printTags(Tag[] tags)
   {
      for (int i=0; i<tags.length; ++i) {
         out.println("Tag #" + (i+1) + ":");
         printTag(tags[i]);
      }
   }
}
