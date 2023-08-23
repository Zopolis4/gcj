/* Copyright (C) 1999, 2000, 2001, 2003, 2005, 2006  Free Software Foundation

   This file is part of libgcj.

This software is copyrighted work licensed under the terms of the
Libgcj License.  Please consult the file "LIBGCJ_LICENSE" for
details.  */

package gnu.gcj.convert; 

import java.nio.charset.Charset;

public abstract class UnicodeToBytes extends IOConverter
{
  /** Buffer to emit bytes to.
   * The locations buf[count] ... buf[buf.length-1] are available. */
  public byte[] buf;
  public int count;

  // The name of the default encoding.
  static String defaultEncoding;

  /* These keep a small cache of encoders for reuse.  The array holds
     the actual encoders.  The currCachePos is the next value we are
     going to replace in the cache.  We don't just throw the data away
     if the cache is full, because if the cache filled up with stuff we
     don't need then the cache would be worthless.  We instead
     circulate through the cache the implement kind of an LRU
     algorithm. */
  private static final int CACHE_SIZE = 4;  // A power of 2 for speed
  private static UnicodeToBytes[] encoderCache
    = new UnicodeToBytes[CACHE_SIZE];
  private static int currCachePos = 0;

  public abstract String getName();

  public static UnicodeToBytes getDefaultEncoder()
  {
    try
      {
	synchronized (UnicodeToBytes.class)
	  {
	    if (defaultEncoding == null)
	      {
		String encoding
		  = canonicalize (System.getProperty("file.encoding",
						     "8859_1"));
		String className = "gnu.gcj.convert.Output_" + encoding;
		try
		  {
		    Class defaultEncodingClass = Class.forName(className);
		    defaultEncoding = encoding;
		  }
		catch (ClassNotFoundException ex)
		  {
		    throw new NoClassDefFoundError("missing default encoding "
						   + encoding + " (class "
						   + className
						   + " not found)");
		  }
	      }
	  }

	return getEncoder (defaultEncoding);
      }
    catch (Throwable ex)
      {
	return new Output_8859_1();
      }
  }

  /** Get a char-stream->byte-stream converter given an encoding name. */
  public static UnicodeToBytes getEncoder (String encoding)
    throws java.io.UnsupportedEncodingException
  {
    /* First hunt in our cache to see if we have a encoder that is
       already allocated. */
    String canonicalEncoding = canonicalize(encoding);
    synchronized (UnicodeToBytes.class)
      {
	int i;
	for (i = 0; i < encoderCache.length; ++i)
	  {
	    if (encoderCache[i] != null
		&& canonicalEncoding.equals(encoderCache[i].getName ()))
	      {
		UnicodeToBytes rv = encoderCache[i];
		encoderCache[i] = null;
		return rv;
	    }
	  }
      }

    String className = "gnu.gcj.convert.Output_" + canonicalEncoding;
    Class encodingClass;
    try 
      { 
	encodingClass = Class.forName(className); 
	return (UnicodeToBytes) encodingClass.newInstance();
      } 
    catch (Throwable ex) 
      { 
	try
          {
	    // We pass the original name to iconv and let it handle
	    // its own aliasing.  Note that we intentionally prefer
	    // iconv over nio.
	    return new Output_iconv (encoding);
          }
        catch (Throwable _)
          {
            // Ignore, and try the next method.
          }
	try
	  {
            // Try using finding java.nio.charset.Charset and using
            // the adaptor.  Use the original name as Charsets have
            // their own canonical names.
            return new CharsetToBytesAdaptor(Charset.forName(encoding));
	  }
	catch (Throwable _)
	  {
	    // Put the original exception in the throwable.
	    throw new java.io.UnsupportedEncodingException(encoding + " ("
							   + ex + ')');
	  }
      }
  }

  public final void setOutput(byte[] buffer, int count)
  {
    this.buf = buffer;
    this.count = count;
  }

  /** Convert chars to bytes.
    * Converted bytes are written to buf, starting at count.
    * @param inbuffer source of characters to convert
    * @param inpos index of initial character in inbuffer to convert
    * @param inlength number of characters to convert
    * @return number of chars converted
    * Also, this.count is increment by the number of bytes converted.
    */
  public abstract int write (char[] inbuffer, int inpos, int inlength);

  /** Convert chars to bytes.
    * Converted bytes are written to buf, starting at count.
    * @param str source of characters to convert
    * @param inpos index of initial character in str to convert
    * @param inlength number of characters to convert
    * @param work if non-null, a buffer than can be used
    * @return number of chars converted
    * Also, this.count is increment by the number of bytes converted.
    */
  public int write (String str, int inpos, int inlength, char[] work)
  {
    if (work == null)
      work = new char[inlength];
    int srcEnd = inpos + (inlength > work.length ? work.length : inlength);
    str.getChars(inpos, srcEnd, work, 0);
    return write(work, 0, srcEnd - inpos);
  }

  /**
   * Returns true when the converter has consumed some bytes that are
   * not yet converted to characters because further continuation
   * bytes are needed.  Defaults to false, should be overridden by
   * decoders that internally store some bytes.
   */
  public boolean havePendingBytes()
  {
    return false;
  }

  /**
   * Users should call this method when the input is coming to an
   * end.  This signals that the next write (which might be
   * zero-length) ought to flush any internal state.
   */
  public void setFinished()
  {
  }

  /** Indicate that the converter is resuable.
   * This class keeps track of converters on a per-encoding basis.
   * When done with an encoder you may call this method to indicate
   * that it can be reused later.
   */
  public void done ()
  {
    synchronized (UnicodeToBytes.class)
      {
	this.buf = null;
	this.count = 0;

	encoderCache[currCachePos] = this;
	currCachePos = (currCachePos + 1) % CACHE_SIZE;
      }
  }
}
