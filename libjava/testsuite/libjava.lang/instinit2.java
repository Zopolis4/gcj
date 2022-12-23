// Class instinit2
// Generated on Wed Feb  2 17:52:49 PST 2000
// The instance initializer throws a checked exception. This is OK
// since the constructors declares it in its `throws' clause -- at
// least that's what the specs are saying.

class instinit2 {

    String buffer = "Oink Oink!";

    {
        System.out.println ("Checking the oink...");
        if (buffer != null)
            throw new Exception ("It just oinked");
    }

    instinit2 () throws Exception
    {
        System.out.println ("Ctor");
    }

    public static void main (String[] arg)
    {
        System.out.println ("Testing class `instinit2'...");
        try {
            System.out.println (new instinit2 ().buffer);
        } catch (Exception e) {
            System.out.println (e.toString());
        }
    }
} 
