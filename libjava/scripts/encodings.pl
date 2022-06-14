# encodings.pl - Download IANA text and compute alias list.
# Assumes you are running this program from gnu/gcj/convert/.
# Output suitable for direct inclusion in IOConverter.java.

# Map IANA canonical names onto our canonical names.
%map = (
	'ANSI_X3.4-1968' => 'ASCII',
	'ISO_8859-1:1987' => '8859_1',
	'UTF-8' => 'UTF8',
	'Shift_JIS' => 'SJIS',
	'Extended_UNIX_Code_Packed_Format_for_Japanese' => 'EUCJIS',
	'UTF16-LE' => 'UnicodeLittle',
	'UTF16-BE' => 'UnicodeBig' 
	);

if ($ARGV[0] eq '')
{
    $file = 'character-sets';
    if (! -f $file)
    {
	# Too painful to figure out how to get Perl to do it.
	system 'wget -o .wget-log http://www.iana.org/assignments/character-sets';
    }
}
else
{
    $file = $ARGV[0];
}

# Include canonical names in the output.
foreach $key (keys %map)
{
    $output{lc ($key)} = $map{$key};
}

open (INPUT, "< $file") || die "couldn't open $file: $!";

$body = 0;
$current = '';
while (<INPUT>)
{
    chop;
    $body = 1 if /^Name:/;
    next unless $body;

    if (/^$/)
    {
	$current = '';
	next;
    }

    ($type, $name) = split (/\s+/);
    # Encoding names are case-insensitive.  We do all processing on
    # the lower-case form.
    my $lower = lc ($name);
    if ($type eq 'Name:')
    {
	$current = $map{$name};
	if ($current)
	{
	    $output{$lower} = $current;
	}
    }
    elsif ($type eq 'Alias:')
    {
	# The IANA list has some ugliness.
	if ($name ne '' && $lower ne 'none' && $current)
	{
	    $output{$lower} = $current;
	}
    }
}

close (INPUT);

foreach $key (sort keys %output)
{
    print "    hash.put (\"$key\", \"$output{$key}\");\n";
}
