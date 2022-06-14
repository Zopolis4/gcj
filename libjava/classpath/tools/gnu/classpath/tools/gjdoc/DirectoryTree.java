/* gnu.classpath.tools.gjdoc.DirectoryTree
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

package gnu.classpath.tools.gjdoc;

import java.io.*;

public class DirectoryTree {

   class FileNode {
      File file;
      FileNode[] subNodes;

      FileNode(File file) {
         this.file=file;
         if (file.isDirectory()) {
            File[] subFiles=file.listFiles();
            subNodes=new FileNode[subFiles.length];
            for (int i=0; i<subFiles.length; ++i) {
               subNodes[i]=new FileNode(subFiles[i]);
            }
         }
      }
   }

   FileNode root;

   DirectoryTree(File path) {

      System.err.print("Scanning "+path.getAbsolutePath()+"... ");

      long now1=System.currentTimeMillis();

      root=new FileNode(path);

      long now2=System.currentTimeMillis();

      System.err.println("took "+(now2-now1)+" ms");
   }
}
