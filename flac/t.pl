use File::Basename;

my ($n, $d) = fileparse('abc.ogg');
print $n, " ", $d, "\n";
