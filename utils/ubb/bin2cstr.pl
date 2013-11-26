#!/usr/bin/perl

use bytes;
binmode(STDIN);

%nc = (
	0, '0', 7, 'a', 8, 'b', 9, 't', 10, 'n', 11, 'v', 12, 'f', 13, 'r',
	34, '"', 39, '\'', 92, '\\'
);

$max_line_sz = 64;
$cur_line_sz = 0;
$total_chars = 0;
$z0 = 0;
$z1 = 0;

sub check_line_end {
	my ($cnt) = @_;
	if (($cur_line_sz + $cnt) > $max_line_sz) {
		printf("\"\n\t\"");
		$cur_line_sz = $cnt;
	} else {
		$cur_line_sz += $cnt;
	}
}

printf("\t\"");
while (1) {
	$_ = getc();
	last unless defined $_;
	$total_chars++;

	my $c = ord($_);
	$z1 = $z0;
	$z0 = ($c == 0);
	if ($c < 127) {
		if (exists $nc{$c}) {
			&check_line_end(2);
			print "\\$nc{$c}";
		} elsif (($c < 32) || $z1) {
			&check_line_end(4);
			printf("\\%03o", $c);
		} else {
			&check_line_end(1);
			print chr($c);
		}
	} else {
		&check_line_end(4);
		printf("\\%03o", $c);
	}
}

printf("\", %d\n", ${total_chars});
