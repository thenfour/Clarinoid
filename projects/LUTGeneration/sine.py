
use Math::Trig ':pi';
$len = 256;
print "const int16_t AudioWaveformSine[257] = {\n";
for ($i=0; $i <= $len; $i++) {
        $f = sin($i / $len * 2 * pi);
        $d = sprintf "%.0f", $f * 32767.0;
#print $d;
        printf "%6d", $d + 0;
        print "," if ($i < $len);
        print "\n" if ($i % 10) == 9;
}
print "\n" unless ($len % 10) == 9;
print "};\n";


