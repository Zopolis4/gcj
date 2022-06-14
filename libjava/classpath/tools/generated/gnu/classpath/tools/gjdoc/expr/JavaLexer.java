// $ANTLR 2.7.7 (20080530): "java-expression.g" -> "JavaLexer.java"$

   package gnu.classpath.tools.gjdoc.expr;

import java.io.InputStream;
import antlr.TokenStreamException;
import antlr.TokenStreamIOException;
import antlr.TokenStreamRecognitionException;
import antlr.CharStreamException;
import antlr.CharStreamIOException;
import antlr.ANTLRException;
import java.io.Reader;
import java.util.Hashtable;
import antlr.CharScanner;
import antlr.InputBuffer;
import antlr.ByteBuffer;
import antlr.CharBuffer;
import antlr.Token;
import antlr.CommonToken;
import antlr.RecognitionException;
import antlr.NoViableAltForCharException;
import antlr.MismatchedCharException;
import antlr.TokenStream;
import antlr.ANTLRHashString;
import antlr.LexerSharedInputState;
import antlr.collections.impl.BitSet;
import antlr.SemanticException;

public class JavaLexer extends antlr.CharScanner implements JavaTokenTypes, TokenStream
 {
public JavaLexer(InputStream in) {
        this(new ByteBuffer(in));
}
public JavaLexer(Reader in) {
        this(new CharBuffer(in));
}
public JavaLexer(InputBuffer ib) {
        this(new LexerSharedInputState(ib));
}
public JavaLexer(LexerSharedInputState state) {
        super(state);
        caseSensitiveLiterals = true;
        setCaseSensitive(true);
        literals = new Hashtable();
        literals.put(new ANTLRHashString("String", this), new Integer(55));
        literals.put(new ANTLRHashString("final", this), new Integer(39));
        literals.put(new ANTLRHashString("false", this), new Integer(84));
        literals.put(new ANTLRHashString("true", this), new Integer(83));
        literals.put(new ANTLRHashString("void", this), new Integer(46));
        literals.put(new ANTLRHashString("float", this), new Integer(52));
        literals.put(new ANTLRHashString("boolean", this), new Integer(47));
        literals.put(new ANTLRHashString("long", this), new Integer(53));
        literals.put(new ANTLRHashString("null", this), new Integer(85));
        literals.put(new ANTLRHashString("short", this), new Integer(50));
        literals.put(new ANTLRHashString("char", this), new Integer(49));
        literals.put(new ANTLRHashString("abstract", this), new Integer(40));
        literals.put(new ANTLRHashString("byte", this), new Integer(48));
        literals.put(new ANTLRHashString("int", this), new Integer(51));
        literals.put(new ANTLRHashString("double", this), new Integer(54));
        literals.put(new ANTLRHashString("strictfp", this), new Integer(41));
}

public Token nextToken() throws TokenStreamException {
        Token theRetToken=null;
tryAgain:
        for (;;) {
                Token _token = null;
                int _ttype = Token.INVALID_TYPE;
                resetText();
                try {   // for char stream error handling
                        try {   // for lexical error handling
                                switch ( LA(1)) {
                                case '?':
                                {
                                        mQUESTION(true);
                                        theRetToken=_returnToken;
                                        break;
                                }
                                case '(':
                                {
                                        mLPAREN(true);
                                        theRetToken=_returnToken;
                                        break;
                                }
                                case ')':
                                {
                                        mRPAREN(true);
                                        theRetToken=_returnToken;
                                        break;
                                }
                                case '[':
                                {
                                        mLBRACK(true);
                                        theRetToken=_returnToken;
                                        break;
                                }
                                case ']':
                                {
                                        mRBRACK(true);
                                        theRetToken=_returnToken;
                                        break;
                                }
                                case '{':
                                {
                                        mLCURLY(true);
                                        theRetToken=_returnToken;
                                        break;
                                }
                                case '}':
                                {
                                        mRCURLY(true);
                                        theRetToken=_returnToken;
                                        break;
                                }
                                case ':':
                                {
                                        mCOLON(true);
                                        theRetToken=_returnToken;
                                        break;
                                }
                                case ',':
                                {
                                        mCOMMA(true);
                                        theRetToken=_returnToken;
                                        break;
                                }
                                case '~':
                                {
                                        mBNOT(true);
                                        theRetToken=_returnToken;
                                        break;
                                }
                                case ';':
                                {
                                        mSEMI(true);
                                        theRetToken=_returnToken;
                                        break;
                                }
                                case '\t':  case '\n':  case '\u000c':  case '\r':
                                case ' ':
                                {
                                        mWS(true);
                                        theRetToken=_returnToken;
                                        break;
                                }
                                case '\'':
                                {
                                        mCHAR_LITERAL(true);
                                        theRetToken=_returnToken;
                                        break;
                                }
                                case '"':
                                {
                                        mSTRING_LITERAL(true);
                                        theRetToken=_returnToken;
                                        break;
                                }
                                case '$':  case 'A':  case 'B':  case 'C':
                                case 'D':  case 'E':  case 'F':  case 'G':
                                case 'H':  case 'I':  case 'J':  case 'K':
                                case 'L':  case 'M':  case 'N':  case 'O':
                                case 'P':  case 'Q':  case 'R':  case 'S':
                                case 'T':  case 'U':  case 'V':  case 'W':
                                case 'X':  case 'Y':  case 'Z':  case '_':
                                case 'a':  case 'b':  case 'c':  case 'd':
                                case 'e':  case 'f':  case 'g':  case 'h':
                                case 'i':  case 'j':  case 'k':  case 'l':
                                case 'm':  case 'n':  case 'o':  case 'p':
                                case 'q':  case 'r':  case 's':  case 't':
                                case 'u':  case 'v':  case 'w':  case 'x':
                                case 'y':  case 'z':
                                {
                                        mIDENT(true);
                                        theRetToken=_returnToken;
                                        break;
                                }
                                case '.':  case '0':  case '1':  case '2':
                                case '3':  case '4':  case '5':  case '6':
                                case '7':  case '8':  case '9':
                                {
                                        mNUM_INT(true);
                                        theRetToken=_returnToken;
                                        break;
                                }
                                default:
                                        if ((LA(1)=='>') && (LA(2)=='>') && (LA(3)=='>') && (LA(4)=='=')) {
                                                mBSR_ASSIGN(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='>') && (LA(2)=='>') && (LA(3)=='=')) {
                                                mSR_ASSIGN(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='>') && (LA(2)=='>') && (LA(3)=='>') && (true)) {
                                                mBSR(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='<') && (LA(2)=='<') && (LA(3)=='=')) {
                                                mSL_ASSIGN(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='=') && (LA(2)=='=')) {
                                                mEQUAL(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='!') && (LA(2)=='=')) {
                                                mNOT_EQUAL(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='/') && (LA(2)=='=')) {
                                                mDIV_ASSIGN(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='+') && (LA(2)=='=')) {
                                                mPLUS_ASSIGN(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='+') && (LA(2)=='+')) {
                                                mINC(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='-') && (LA(2)=='=')) {
                                                mMINUS_ASSIGN(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='-') && (LA(2)=='-')) {
                                                mDEC(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='*') && (LA(2)=='=')) {
                                                mSTAR_ASSIGN(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='%') && (LA(2)=='=')) {
                                                mMOD_ASSIGN(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='>') && (LA(2)=='>') && (true)) {
                                                mSR(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='>') && (LA(2)=='=')) {
                                                mGE(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='<') && (LA(2)=='<') && (true)) {
                                                mSL(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='<') && (LA(2)=='=')) {
                                                mLE(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='^') && (LA(2)=='=')) {
                                                mBXOR_ASSIGN(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='|') && (LA(2)=='=')) {
                                                mBOR_ASSIGN(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='|') && (LA(2)=='|')) {
                                                mLOR(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='&') && (LA(2)=='=')) {
                                                mBAND_ASSIGN(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='&') && (LA(2)=='&')) {
                                                mLAND(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='/') && (LA(2)=='/')) {
                                                mSL_COMMIT(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='/') && (LA(2)=='*')) {
                                                mML_COMMENT(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='=') && (true)) {
                                                mASSIGN(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='!') && (true)) {
                                                mLNOT(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='/') && (true)) {
                                                mDIV(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='+') && (true)) {
                                                mPLUS(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='-') && (true)) {
                                                mMINUS(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='*') && (true)) {
                                                mSTAR(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='%') && (true)) {
                                                mMOD(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='>') && (true)) {
                                                mGT(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='<') && (true)) {
                                                mLT(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='^') && (true)) {
                                                mBXOR(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='|') && (true)) {
                                                mBOR(true);
                                                theRetToken=_returnToken;
                                        }
                                        else if ((LA(1)=='&') && (true)) {
                                                mBAND(true);
                                                theRetToken=_returnToken;
                                        }
                                else {
                                        if (LA(1)==EOF_CHAR) {uponEOF(); _returnToken = makeToken(Token.EOF_TYPE);}
                                else {throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());}
                                }
                                }
                                if ( _returnToken==null ) continue tryAgain; // found SKIP token
                                _ttype = _returnToken.getType();
                                _returnToken.setType(_ttype);
                                return _returnToken;
                        }
                        catch (RecognitionException e) {
                                throw new TokenStreamRecognitionException(e);
                        }
                }
                catch (CharStreamException cse) {
                        if ( cse instanceof CharStreamIOException ) {
                                throw new TokenStreamIOException(((CharStreamIOException)cse).io);
                        }
                        else {
                                throw new TokenStreamException(cse.getMessage());
                        }
                }
        }
}

        public final void mQUESTION(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = QUESTION;
                int _saveIndex;

                match('?');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mLPAREN(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = LPAREN;
                int _saveIndex;

                match('(');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mRPAREN(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = RPAREN;
                int _saveIndex;

                match(')');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mLBRACK(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = LBRACK;
                int _saveIndex;

                match('[');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mRBRACK(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = RBRACK;
                int _saveIndex;

                match(']');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mLCURLY(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = LCURLY;
                int _saveIndex;

                match('{');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mRCURLY(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = RCURLY;
                int _saveIndex;

                match('}');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mCOLON(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = COLON;
                int _saveIndex;

                match(':');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mCOMMA(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = COMMA;
                int _saveIndex;

                match(',');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mASSIGN(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = ASSIGN;
                int _saveIndex;

                match('=');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mEQUAL(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = EQUAL;
                int _saveIndex;

                match("==");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mLNOT(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = LNOT;
                int _saveIndex;

                match('!');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mBNOT(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = BNOT;
                int _saveIndex;

                match('~');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mNOT_EQUAL(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = NOT_EQUAL;
                int _saveIndex;

                match("!=");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mDIV(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = DIV;
                int _saveIndex;

                match('/');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mDIV_ASSIGN(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = DIV_ASSIGN;
                int _saveIndex;

                match("/=");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mPLUS(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = PLUS;
                int _saveIndex;

                match('+');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mPLUS_ASSIGN(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = PLUS_ASSIGN;
                int _saveIndex;

                match("+=");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mINC(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = INC;
                int _saveIndex;

                match("++");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mMINUS(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = MINUS;
                int _saveIndex;

                match('-');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mMINUS_ASSIGN(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = MINUS_ASSIGN;
                int _saveIndex;

                match("-=");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mDEC(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = DEC;
                int _saveIndex;

                match("--");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mSTAR(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = STAR;
                int _saveIndex;

                match('*');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mSTAR_ASSIGN(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = STAR_ASSIGN;
                int _saveIndex;

                match("*=");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mMOD(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = MOD;
                int _saveIndex;

                match('%');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mMOD_ASSIGN(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = MOD_ASSIGN;
                int _saveIndex;

                match("%=");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mSR(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = SR;
                int _saveIndex;

                match(">>");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mSR_ASSIGN(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = SR_ASSIGN;
                int _saveIndex;

                match(">>=");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mBSR(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = BSR;
                int _saveIndex;

                match(">>>");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mBSR_ASSIGN(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = BSR_ASSIGN;
                int _saveIndex;

                match(">>>=");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mGE(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = GE;
                int _saveIndex;

                match(">=");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mGT(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = GT;
                int _saveIndex;

                match(">");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mSL(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = SL;
                int _saveIndex;

                match("<<");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mSL_ASSIGN(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = SL_ASSIGN;
                int _saveIndex;

                match("<<=");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mLE(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = LE;
                int _saveIndex;

                match("<=");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mLT(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = LT;
                int _saveIndex;

                match('<');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mBXOR(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = BXOR;
                int _saveIndex;

                match('^');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mBXOR_ASSIGN(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = BXOR_ASSIGN;
                int _saveIndex;

                match("^=");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mBOR(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = BOR;
                int _saveIndex;

                match('|');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mBOR_ASSIGN(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = BOR_ASSIGN;
                int _saveIndex;

                match("|=");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mLOR(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = LOR;
                int _saveIndex;

                match("||");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mBAND(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = BAND;
                int _saveIndex;

                match('&');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mBAND_ASSIGN(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = BAND_ASSIGN;
                int _saveIndex;

                match("&=");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mLAND(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = LAND;
                int _saveIndex;

                match("&&");
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mSEMI(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = SEMI;
                int _saveIndex;

                match(';');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mWS(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = WS;
                int _saveIndex;

                {
                int _cnt105=0;
                _loop105:
                do {
                        switch ( LA(1)) {
                        case ' ':
                        {
                                match(' ');
                                break;
                        }
                        case '\t':
                        {
                                match('\t');
                                break;
                        }
                        case '\u000c':
                        {
                                match('\f');
                                break;
                        }
                        case '\n':  case '\r':
                        {
                                {
                                if ((LA(1)=='\r') && (LA(2)=='\n') && (true) && (true)) {
                                        match("\r\n");
                                }
                                else if ((LA(1)=='\r') && (true) && (true) && (true)) {
                                        match('\r');
                                }
                                else if ((LA(1)=='\n')) {
                                        match('\n');
                                }
                                else {
                                        throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
                                }

                                }
                                if ( inputState.guessing==0 ) {
                                        newline();
                                }
                                break;
                        }
                        default:
                        {
                                if ( _cnt105>=1 ) { break _loop105; } else {throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());}
                        }
                        }
                        _cnt105++;
                } while (true);
                }
                if ( inputState.guessing==0 ) {
                        _ttype = Token.SKIP;
                }
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mSL_COMMIT(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = SL_COMMIT;
                int _saveIndex;

                match("//");
                {
                _loop109:
                do {
                        if ((_tokenSet_0.member(LA(1)))) {
                                {
                                match(_tokenSet_0);
                                }
                        }
                        else {
                                break _loop109;
                        }

                } while (true);
                }
                {
                switch ( LA(1)) {
                case '\n':
                {
                        match('\n');
                        break;
                }
                case '\r':
                {
                        match('\r');
                        {
                        if ((LA(1)=='\n')) {
                                match('\n');
                        }
                        else {
                        }

                        }
                        break;
                }
                default:
                {
                        throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
                }
                }
                }
                if ( inputState.guessing==0 ) {
                        _ttype = Token.SKIP; newline();
                }
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mML_COMMENT(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = ML_COMMENT;
                int _saveIndex;

                match("/*");
                {
                _loop115:
                do {
                        if ((LA(1)=='\r') && (LA(2)=='\n') && ((LA(3) >= '\u0003' && LA(3) <= '\uffff')) && ((LA(4) >= '\u0003' && LA(4) <= '\uffff'))) {
                                match('\r');
                                match('\n');
                                if ( inputState.guessing==0 ) {
                                        newline();
                                }
                        }
                        else if (((LA(1)=='*') && ((LA(2) >= '\u0003' && LA(2) <= '\uffff')) && ((LA(3) >= '\u0003' && LA(3) <= '\uffff')))&&( LA(2)!='/' )) {
                                match('*');
                        }
                        else if ((LA(1)=='\r') && ((LA(2) >= '\u0003' && LA(2) <= '\uffff')) && ((LA(3) >= '\u0003' && LA(3) <= '\uffff')) && (true)) {
                                match('\r');
                                if ( inputState.guessing==0 ) {
                                        newline();
                                }
                        }
                        else if ((LA(1)=='\n')) {
                                match('\n');
                                if ( inputState.guessing==0 ) {
                                        newline();
                                }
                        }
                        else if ((_tokenSet_1.member(LA(1)))) {
                                {
                                match(_tokenSet_1);
                                }
                        }
                        else {
                                break _loop115;
                        }

                } while (true);
                }
                match("*/");
                if ( inputState.guessing==0 ) {
                        _ttype = Token.SKIP;
                }
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mCHAR_LITERAL(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = CHAR_LITERAL;
                int _saveIndex;

                match('\'');
                {
                if ((LA(1)=='\\')) {
                        mESC(false);
                }
                else if ((_tokenSet_2.member(LA(1)))) {
                        {
                        match(_tokenSet_2);
                        }
                }
                else {
                        throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
                }

                }
                match('\'');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        protected final void mESC(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = ESC;
                int _saveIndex;

                match('\\');
                {
                switch ( LA(1)) {
                case 'n':
                {
                        match('n');
                        break;
                }
                case 'r':
                {
                        match('r');
                        break;
                }
                case 't':
                {
                        match('t');
                        break;
                }
                case 'b':
                {
                        match('b');
                        break;
                }
                case 'f':
                {
                        match('f');
                        break;
                }
                case '"':
                {
                        match('"');
                        break;
                }
                case '\'':
                {
                        match('\'');
                        break;
                }
                case '\\':
                {
                        match('\\');
                        break;
                }
                case 'u':
                {
                        {
                        int _cnt126=0;
                        _loop126:
                        do {
                                if ((LA(1)=='u')) {
                                        match('u');
                                }
                                else {
                                        if ( _cnt126>=1 ) { break _loop126; } else {throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());}
                                }

                                _cnt126++;
                        } while (true);
                        }
                        mHEX_DIGIT(false);
                        mHEX_DIGIT(false);
                        mHEX_DIGIT(false);
                        mHEX_DIGIT(false);
                        break;
                }
                case '0':  case '1':  case '2':  case '3':
                {
                        matchRange('0','3');
                        {
                        if (((LA(1) >= '0' && LA(1) <= '7')) && (_tokenSet_0.member(LA(2))) && (true) && (true)) {
                                matchRange('0','7');
                                {
                                if (((LA(1) >= '0' && LA(1) <= '7')) && (_tokenSet_0.member(LA(2))) && (true) && (true)) {
                                        matchRange('0','7');
                                }
                                else if ((_tokenSet_0.member(LA(1))) && (true) && (true) && (true)) {
                                }
                                else {
                                        throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
                                }

                                }
                        }
                        else if ((_tokenSet_0.member(LA(1))) && (true) && (true) && (true)) {
                        }
                        else {
                                throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
                        }

                        }
                        break;
                }
                case '4':  case '5':  case '6':  case '7':
                {
                        matchRange('4','7');
                        {
                        if (((LA(1) >= '0' && LA(1) <= '7')) && (_tokenSet_0.member(LA(2))) && (true) && (true)) {
                                matchRange('0','7');
                        }
                        else if ((_tokenSet_0.member(LA(1))) && (true) && (true) && (true)) {
                        }
                        else {
                                throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
                        }

                        }
                        break;
                }
                default:
                {
                        throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
                }
                }
                }
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mSTRING_LITERAL(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = STRING_LITERAL;
                int _saveIndex;

                match('"');
                {
                _loop122:
                do {
                        if ((LA(1)=='\\')) {
                                mESC(false);
                        }
                        else if ((_tokenSet_3.member(LA(1)))) {
                                {
                                match(_tokenSet_3);
                                }
                        }
                        else {
                                break _loop122;
                        }

                } while (true);
                }
                match('"');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        protected final void mHEX_DIGIT(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = HEX_DIGIT;
                int _saveIndex;

                {
                switch ( LA(1)) {
                case '0':  case '1':  case '2':  case '3':
                case '4':  case '5':  case '6':  case '7':
                case '8':  case '9':
                {
                        matchRange('0','9');
                        break;
                }
                case 'A':  case 'B':  case 'C':  case 'D':
                case 'E':  case 'F':
                {
                        matchRange('A','F');
                        break;
                }
                case 'a':  case 'b':  case 'c':  case 'd':
                case 'e':  case 'f':
                {
                        matchRange('a','f');
                        break;
                }
                default:
                {
                        throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
                }
                }
                }
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        protected final void mVOCAB(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = VOCAB;
                int _saveIndex;

                matchRange('\3','\377');
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mIDENT(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = IDENT;
                int _saveIndex;

                {
                switch ( LA(1)) {
                case 'a':  case 'b':  case 'c':  case 'd':
                case 'e':  case 'f':  case 'g':  case 'h':
                case 'i':  case 'j':  case 'k':  case 'l':
                case 'm':  case 'n':  case 'o':  case 'p':
                case 'q':  case 'r':  case 's':  case 't':
                case 'u':  case 'v':  case 'w':  case 'x':
                case 'y':  case 'z':
                {
                        matchRange('a','z');
                        break;
                }
                case 'A':  case 'B':  case 'C':  case 'D':
                case 'E':  case 'F':  case 'G':  case 'H':
                case 'I':  case 'J':  case 'K':  case 'L':
                case 'M':  case 'N':  case 'O':  case 'P':
                case 'Q':  case 'R':  case 'S':  case 'T':
                case 'U':  case 'V':  case 'W':  case 'X':
                case 'Y':  case 'Z':
                {
                        matchRange('A','Z');
                        break;
                }
                case '_':
                {
                        match('_');
                        break;
                }
                case '$':
                {
                        match('$');
                        break;
                }
                default:
                {
                        throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
                }
                }
                }
                {
                _loop136:
                do {
                        switch ( LA(1)) {
                        case 'a':  case 'b':  case 'c':  case 'd':
                        case 'e':  case 'f':  case 'g':  case 'h':
                        case 'i':  case 'j':  case 'k':  case 'l':
                        case 'm':  case 'n':  case 'o':  case 'p':
                        case 'q':  case 'r':  case 's':  case 't':
                        case 'u':  case 'v':  case 'w':  case 'x':
                        case 'y':  case 'z':
                        {
                                matchRange('a','z');
                                break;
                        }
                        case 'A':  case 'B':  case 'C':  case 'D':
                        case 'E':  case 'F':  case 'G':  case 'H':
                        case 'I':  case 'J':  case 'K':  case 'L':
                        case 'M':  case 'N':  case 'O':  case 'P':
                        case 'Q':  case 'R':  case 'S':  case 'T':
                        case 'U':  case 'V':  case 'W':  case 'X':
                        case 'Y':  case 'Z':
                        {
                                matchRange('A','Z');
                                break;
                        }
                        case '_':
                        {
                                match('_');
                                break;
                        }
                        case '0':  case '1':  case '2':  case '3':
                        case '4':  case '5':  case '6':  case '7':
                        case '8':  case '9':
                        {
                                matchRange('0','9');
                                break;
                        }
                        case '$':
                        {
                                match('$');
                                break;
                        }
                        default:
                        {
                                break _loop136;
                        }
                        }
                } while (true);
                }
                _ttype = testLiteralsTable(_ttype);
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        public final void mNUM_INT(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = NUM_INT;
                int _saveIndex;
                Token f1=null;
                Token f2=null;
                Token f3=null;
                Token f4=null;
                boolean isDecimal=false; Token t=null;

                switch ( LA(1)) {
                case '.':
                {
                        match('.');
                        if ( inputState.guessing==0 ) {
                                _ttype = DOT;
                        }
                        {
                        if (((LA(1) >= '0' && LA(1) <= '9'))) {
                                {
                                int _cnt140=0;
                                _loop140:
                                do {
                                        if (((LA(1) >= '0' && LA(1) <= '9'))) {
                                                matchRange('0','9');
                                        }
                                        else {
                                                if ( _cnt140>=1 ) { break _loop140; } else {throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());}
                                        }

                                        _cnt140++;
                                } while (true);
                                }
                                {
                                if ((LA(1)=='E'||LA(1)=='e')) {
                                        mEXPONENT(false);
                                }
                                else {
                                }

                                }
                                {
                                if ((LA(1)=='D'||LA(1)=='F'||LA(1)=='d'||LA(1)=='f')) {
                                        mFLOAT_SUFFIX(true);
                                        f1=_returnToken;
                                        if ( inputState.guessing==0 ) {
                                                t=f1;
                                        }
                                }
                                else {
                                }

                                }
                                if ( inputState.guessing==0 ) {

                                                                        if (t != null && t.getText().toUpperCase().indexOf('F')>=0) {
                                                _ttype = NUM_FLOAT;
                                                                        }
                                                                        else {
                                                _ttype = NUM_DOUBLE; // assume double
                                                                        }

                                }
                        }
                        else {
                        }

                        }
                        break;
                }
                case '0':  case '1':  case '2':  case '3':
                case '4':  case '5':  case '6':  case '7':
                case '8':  case '9':
                {
                        {
                        switch ( LA(1)) {
                        case '0':
                        {
                                match('0');
                                if ( inputState.guessing==0 ) {
                                        isDecimal = true;
                                }
                                {
                                if ((LA(1)=='X'||LA(1)=='x')) {
                                        {
                                        switch ( LA(1)) {
                                        case 'x':
                                        {
                                                match('x');
                                                break;
                                        }
                                        case 'X':
                                        {
                                                match('X');
                                                break;
                                        }
                                        default:
                                        {
                                                throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
                                        }
                                        }
                                        }
                                        {
                                        int _cnt147=0;
                                        _loop147:
                                        do {
                                                if ((_tokenSet_4.member(LA(1))) && (true) && (true) && (true)) {
                                                        mHEX_DIGIT(false);
                                                }
                                                else {
                                                        if ( _cnt147>=1 ) { break _loop147; } else {throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());}
                                                }

                                                _cnt147++;
                                        } while (true);
                                        }
                                }
                                else {
                                        boolean synPredMatched152 = false;
                                        if ((((LA(1) >= '0' && LA(1) <= '9')) && (true) && (true) && (true))) {
                                                int _m152 = mark();
                                                synPredMatched152 = true;
                                                inputState.guessing++;
                                                try {
                                                        {
                                                        {
                                                        int _cnt150=0;
                                                        _loop150:
                                                        do {
                                                                if (((LA(1) >= '0' && LA(1) <= '9'))) {
                                                                        matchRange('0','9');
                                                                }
                                                                else {
                                                                        if ( _cnt150>=1 ) { break _loop150; } else {throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());}
                                                                }

                                                                _cnt150++;
                                                        } while (true);
                                                        }
                                                        {
                                                        switch ( LA(1)) {
                                                        case '.':
                                                        {
                                                                match('.');
                                                                break;
                                                        }
                                                        case 'E':  case 'e':
                                                        {
                                                                mEXPONENT(false);
                                                                break;
                                                        }
                                                        case 'D':  case 'F':  case 'd':  case 'f':
                                                        {
                                                                mFLOAT_SUFFIX(false);
                                                                break;
                                                        }
                                                        default:
                                                        {
                                                                throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
                                                        }
                                                        }
                                                        }
                                                        }
                                                }
                                                catch (RecognitionException pe) {
                                                        synPredMatched152 = false;
                                                }
                                                rewind(_m152);
inputState.guessing--;
                                        }
                                        if ( synPredMatched152 ) {
                                                {
                                                int _cnt154=0;
                                                _loop154:
                                                do {
                                                        if (((LA(1) >= '0' && LA(1) <= '9'))) {
                                                                matchRange('0','9');
                                                        }
                                                        else {
                                                                if ( _cnt154>=1 ) { break _loop154; } else {throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());}
                                                        }

                                                        _cnt154++;
                                                } while (true);
                                                }
                                        }
                                        else if (((LA(1) >= '0' && LA(1) <= '7')) && (true) && (true) && (true)) {
                                                {
                                                int _cnt156=0;
                                                _loop156:
                                                do {
                                                        if (((LA(1) >= '0' && LA(1) <= '7'))) {
                                                                matchRange('0','7');
                                                        }
                                                        else {
                                                                if ( _cnt156>=1 ) { break _loop156; } else {throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());}
                                                        }

                                                        _cnt156++;
                                                } while (true);
                                                }
                                        }
                                        else {
                                        }
                                        }
                                        }
                                        break;
                                }
                                case '1':  case '2':  case '3':  case '4':
                                case '5':  case '6':  case '7':  case '8':
                                case '9':
                                {
                                        {
                                        matchRange('1','9');
                                        }
                                        {
                                        _loop159:
                                        do {
                                                if (((LA(1) >= '0' && LA(1) <= '9'))) {
                                                        matchRange('0','9');
                                                }
                                                else {
                                                        break _loop159;
                                                }

                                        } while (true);
                                        }
                                        if ( inputState.guessing==0 ) {
                                                isDecimal=true;
                                        }
                                        break;
                                }
                                default:
                                {
                                        throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
                                }
                                }
                                }
                                {
                                if ((LA(1)=='L'||LA(1)=='l')) {
                                        {
                                        switch ( LA(1)) {
                                        case 'l':
                                        {
                                                match('l');
                                                break;
                                        }
                                        case 'L':
                                        {
                                                match('L');
                                                break;
                                        }
                                        default:
                                        {
                                                throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
                                        }
                                        }
                                        }
                                        if ( inputState.guessing==0 ) {
                                                _ttype = NUM_LONG;
                                        }
                                }
                                else if (((LA(1)=='.'||LA(1)=='D'||LA(1)=='E'||LA(1)=='F'||LA(1)=='d'||LA(1)=='e'||LA(1)=='f'))&&(isDecimal)) {
                                        {
                                        switch ( LA(1)) {
                                        case '.':
                                        {
                                                match('.');
                                                {
                                                _loop164:
                                                do {
                                                        if (((LA(1) >= '0' && LA(1) <= '9'))) {
                                                                matchRange('0','9');
                                                        }
                                                        else {
                                                                break _loop164;
                                                        }

                                                } while (true);
                                                }
                                                {
                                                if ((LA(1)=='E'||LA(1)=='e')) {
                                                        mEXPONENT(false);
                                                }
                                                else {
                                                }

                                                }
                                                {
                                                if ((LA(1)=='D'||LA(1)=='F'||LA(1)=='d'||LA(1)=='f')) {
                                                        mFLOAT_SUFFIX(true);
                                                        f2=_returnToken;
                                                        if ( inputState.guessing==0 ) {
                                                                t=f2;
                                                        }
                                                }
                                                else {
                                                }

                                                }
                                                break;
                                        }
                                        case 'E':  case 'e':
                                        {
                                                mEXPONENT(false);
                                                {
                                                if ((LA(1)=='D'||LA(1)=='F'||LA(1)=='d'||LA(1)=='f')) {
                                                        mFLOAT_SUFFIX(true);
                                                        f3=_returnToken;
                                                        if ( inputState.guessing==0 ) {
                                                                t=f3;
                                                        }
                                                }
                                                else {
                                                }

                                                }
                                                break;
                                        }
                                        case 'D':  case 'F':  case 'd':  case 'f':
                                        {
                                                mFLOAT_SUFFIX(true);
                                                f4=_returnToken;
                                                if ( inputState.guessing==0 ) {
                                                        t=f4;
                                                }
                                                break;
                                        }
                                        default:
                                        {
                                                throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
                                        }
                                        }
                                        }
                                        if ( inputState.guessing==0 ) {

                                                                        if (t != null && t.getText().toUpperCase() .indexOf('F') >= 0) {
                                                _ttype = NUM_FLOAT;
                                                                        }
                                                else {
                                                                        _ttype = NUM_DOUBLE; // assume double
                                                                        }

                                        }
                                }
                                else {
                                }

                                }
                                break;
                        }
                        default:
                        {
                                throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
                        }
                        }
                        if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                                _token = makeToken(_ttype);
                                _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                        }
                        _returnToken = _token;
                }

        protected final void mEXPONENT(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = EXPONENT;
                int _saveIndex;

                {
                switch ( LA(1)) {
                case 'e':
                {
                        match('e');
                        break;
                }
                case 'E':
                {
                        match('E');
                        break;
                }
                default:
                {
                        throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
                }
                }
                }
                {
                switch ( LA(1)) {
                case '+':
                {
                        match('+');
                        break;
                }
                case '-':
                {
                        match('-');
                        break;
                }
                case '0':  case '1':  case '2':  case '3':
                case '4':  case '5':  case '6':  case '7':
                case '8':  case '9':
                {
                        break;
                }
                default:
                {
                        throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
                }
                }
                }
                {
                int _cnt172=0;
                _loop172:
                do {
                        if (((LA(1) >= '0' && LA(1) <= '9'))) {
                                matchRange('0','9');
                        }
                        else {
                                if ( _cnt172>=1 ) { break _loop172; } else {throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());}
                        }

                        _cnt172++;
                } while (true);
                }
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }

        protected final void mFLOAT_SUFFIX(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
                int _ttype; Token _token=null; int _begin=text.length();
                _ttype = FLOAT_SUFFIX;
                int _saveIndex;

                switch ( LA(1)) {
                case 'f':
                {
                        match('f');
                        break;
                }
                case 'F':
                {
                        match('F');
                        break;
                }
                case 'd':
                {
                        match('d');
                        break;
                }
                case 'D':
                {
                        match('D');
                        break;
                }
                default:
                {
                        throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
                }
                }
                if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
                        _token = makeToken(_ttype);
                        _token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
                }
                _returnToken = _token;
        }


        private static final long[] mk_tokenSet_0() {
                long[] data = new long[2048];
                data[0]=-9224L;
                for (int i = 1; i<=1023; i++) { data[i]=-1L; }
                return data;
        }
        public static final BitSet _tokenSet_0 = new BitSet(mk_tokenSet_0());
        private static final long[] mk_tokenSet_1() {
                long[] data = new long[2048];
                data[0]=-4398046520328L;
                for (int i = 1; i<=1023; i++) { data[i]=-1L; }
                return data;
        }
        public static final BitSet _tokenSet_1 = new BitSet(mk_tokenSet_1());
        private static final long[] mk_tokenSet_2() {
                long[] data = new long[2048];
                data[0]=-549755823112L;
                data[1]=-268435457L;
                for (int i = 2; i<=1023; i++) { data[i]=-1L; }
                return data;
        }
        public static final BitSet _tokenSet_2 = new BitSet(mk_tokenSet_2());
        private static final long[] mk_tokenSet_3() {
                long[] data = new long[2048];
                data[0]=-17179878408L;
                data[1]=-268435457L;
                for (int i = 2; i<=1023; i++) { data[i]=-1L; }
                return data;
        }
        public static final BitSet _tokenSet_3 = new BitSet(mk_tokenSet_3());
        private static final long[] mk_tokenSet_4() {
                long[] data = new long[1025];
                data[0]=287948901175001088L;
                data[1]=541165879422L;
                return data;
        }
        public static final BitSet _tokenSet_4 = new BitSet(mk_tokenSet_4());

        }
