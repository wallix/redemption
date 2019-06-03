#!/usr/bin/awk  -OSf
# colout '-po' '-ER' '^rdpproxy: (INFO [^-]+-- [-=]{3,}>? )([^-=<]*)(.*)?' 'b' 'g' 'b' '--' '-ERci1' '^rdpproxy: ((INFO)|(WARNING)|(ERR)|(NOTICE)|(DEBUG|EMERG|ALERT|CRIT))[^-]+-- ' 'n' 'b' 'Y' 'R' 'c' 'W' '--' '-cER' '(#[0-9]+)()' '+v,W' '+rv' '+u' '+ru' '--' '-c' '(src/[^:]+|/[^:]+):([:0-9]+)' 'c' 'Y' '--' '-c' '(Assertion) `(.*)'\'' failed.' 'e7,o' 'R' '--' '-c' '^rdpproxy: (\[RDP )(Session|Proxy\]) ' '137' '--' '-cri5' '( type)="([^"]+)"|^([^=]+)="((\\"|[^"])+)"' 'lr' 'lm' 'lb' '243' '--' '-c' '(.+) in (/.*)' 'w' 'i,da' '--' '-cER' '^SUMMARY:' 'r'

# created mer. mai 15 14:26:52 2019

# gawk profile, created Wed May 15 14:26:52 2019

# BEGIN rule(s)

BEGIN {
	esc_reset = "\033[0m"
	colors0[0] = ";34"
	colors0[1] = ";32"
	colors0[2] = ";34"
	nb_colors0 = 3
	colors1[0] = ";0"
	colors1[1] = ";34"
	colors1[2] = ";33;1"
	colors1[3] = ";31;1"
	colors1[4] = ";36"
	colors1[5] = ";97;1"
	nb_colors1 = 6
	colors2[0] = "7;97;1"
	colors2[1] = "27"
	colors2[2] = "4"
	colors2[3] = "24"
	nb_colors2 = 4
	colors3[0] = ";36"
	colors3[1] = ";33;1"
	nb_colors3 = 2
	colors4[0] = ";37;1"
	colors4[1] = ";31;1"
	nb_colors4 = 2
	colors6[0] = ";91"
	colors6[1] = ";95"
	colors6[2] = ";94"
	colors6[3] = ";38;5;243"
	nb_colors6 = 4
	colors7[0] = ";97"
	colors7[1] = ";3;90"
	nb_colors7 = 2
}

# Rule(s)

{
	s = ""
	if (match($0, /^rdpproxy: (INFO [^-]+-- [-=]{3,}>? )([^-=<]*)(.*)?/, a)) {
		n = length(a) / 3
		if (n == 1) {
			i = 0
			ic = 0
			s = s substr($0, 0, RSTART - 1) "\033[" colors0[ic % nb_colors0] "m" a[i]
		} else {
			p = 1
			for (i = 1; i < n; ++i) {
				start = a[i, "start"]
				if (start == null) {
					++n
					continue
				}
				ic = i - 1
				s = s substr($0, p, start - p) "\033[" colors0[ic % nb_colors0] "m" a[i]
				p = start + a[i, "length"]
			}
			s = s substr($0, p, RSTART + RLENGTH - p)
		}
		$0 = substr($0, RLENGTH + RSTART) esc_reset
	}
	if (match($0, /^rdpproxy: ((INFO)|(WARNING)|(ERR)|(NOTICE)|(DEBUG|EMERG|ALERT|CRIT))[^-]+-- /, a)) {
		n = length(a) / 3
		if (n == 1) {
			i = 0
			ic = 0
			s = s substr($0, 0, RSTART - 1) "\033[" colors1[ic % nb_colors1] "m" a[i]
		} else {
			p = 1
			for (i = 1; i < n; ++i) {
				if (i == 1) {
					continue
				}
				start = a[i, "start"]
				if (start == null) {
					++n
					continue
				}
				ic = i - 1
				s = s substr($0, p, start - p) "\033[" colors1[ic % nb_colors1] "m" a[i]
				p = start + a[i, "length"]
			}
			s = s substr($0, p, RSTART + RLENGTH - p)
		}
		$0 = substr($0, RLENGTH + RSTART) esc_reset
	}
	if (match($0, /(#[0-9]+)()/, a)) {
		n = length(a) / 3
		if (n == 1) {
			i = 0
			ic = 0
			s = s substr($0, 0, RSTART - 1) "\033[" colors2[ic % nb_colors2] "m" a[i]
		} else {
			p = 1
			for (i = 1; i < n; ++i) {
				start = a[i, "start"]
				if (start == null) {
					++n
					continue
				}
				ic = i - 1
				s = s substr($0, p, start - p) "\033[" colors2[ic % nb_colors2] "m" a[i]
				p = start + a[i, "length"]
			}
			s = s substr($0, p, RSTART + RLENGTH - p)
		}
		$0 = substr($0, RLENGTH + RSTART) esc_reset
	}
	if (match($0, /(src\/[^:]+|\/[^:]+):([:0-9]+)/, a)) {
		n = length(a) / 3
		if (n == 1) {
			i = 0
			ic = 0
			s = s substr($0, 0, RSTART - 1) "\033[" colors3[ic % nb_colors3] "m" a[i] esc_reset
		} else {
			p = 1
			for (i = 1; i < n; ++i) {
				start = a[i, "start"]
				if (start == null) {
					++n
					continue
				}
				ic = i - 1
				s = s substr($0, p, start - p) "\033[" colors3[ic % nb_colors3] "m" a[i] esc_reset
				p = start + a[i, "length"]
			}
			s = s substr($0, p, RSTART + RLENGTH - p)
		}
		$0 = substr($0, RLENGTH + RSTART)
	}
	if (match($0, /(Assertion) `(.*)' failed./, a)) {
		n = length(a) / 3
		if (n == 1) {
			i = 0
			ic = 0
			s = s substr($0, 0, RSTART - 1) "\033[" colors4[ic % nb_colors4] "m" a[i] esc_reset
		} else {
			p = 1
			for (i = 1; i < n; ++i) {
				start = a[i, "start"]
				if (start == null) {
					++n
					continue
				}
				ic = i - 1
				s = s substr($0, p, start - p) "\033[" colors4[ic % nb_colors4] "m" a[i] esc_reset
				p = start + a[i, "length"]
			}
			s = s substr($0, p, RSTART + RLENGTH - p)
		}
		$0 = substr($0, RLENGTH + RSTART)
	}
	if (match($0, /^rdpproxy: (\[RDP )(Session|Proxy\]) /, a)) {
		c = ";38;5;137"
		n = length(a) / 3
		if (n == 1) {
			i = 0
			s = s substr($0, 0, RSTART - 1) "\033[" c "m" a[i] esc_reset
		} else {
			p = 1
			for (i = 1; i < n; ++i) {
				start = a[i, "start"]
				if (start == null) {
					++n
					continue
				}
				s = s substr($0, p, start - p) "\033[" c "m" a[i] esc_reset
				p = start + a[i, "length"]
			}
			s = s substr($0, p, RSTART + RLENGTH - p)
		}
		$0 = substr($0, RLENGTH + RSTART)
	}
	if (match($0, /( type)="([^"]+)"|^([^=]+)="((\\"|[^"])+)"/, a)) {
		do {
			n = length(a) / 3
			if (n == 1) {
				i = 0
				ic = 0
				s = s substr($0, 0, RSTART - 1) "\033[" colors6[ic % nb_colors6] "m" a[i] esc_reset
			} else {
				p = 1
				for (i = 1; i < n; ++i) {
					if (i == 5) {
						continue
					}
					start = a[i, "start"]
					if (start == null) {
						++n
						continue
					}
					ic = i - 1
					s = s substr($0, p, start - p) "\033[" colors6[ic % nb_colors6] "m" a[i] esc_reset
					p = start + a[i, "length"]
				}
				s = s substr($0, p, RSTART + RLENGTH - p)
			}
			if (RSTART + RLENGTH == 1) {
				break
			}
			$0 = substr($0, RSTART + RLENGTH)
		} while (match($0, /( type)="([^"]+)"|^([^=]+)="((\\"|[^"])+)"/, a))
		$0 = substr($0, RLENGTH + RSTART)
	}
	if (match($0, /(.+) in (\/.*)/, a)) {
		n = length(a) / 3
		if (n == 1) {
			i = 0
			ic = 0
			s = s substr($0, 0, RSTART - 1) "\033[" colors7[ic % nb_colors7] "m" a[i] esc_reset
		} else {
			p = 1
			for (i = 1; i < n; ++i) {
				start = a[i, "start"]
				if (start == null) {
					++n
					continue
				}
				ic = i - 1
				s = s substr($0, p, start - p) "\033[" colors7[ic % nb_colors7] "m" a[i] esc_reset
				p = start + a[i, "length"]
			}
			s = s substr($0, p, RSTART + RLENGTH - p)
		}
		$0 = substr($0, RLENGTH + RSTART)
	}
	if (match($0, /^SUMMARY:/, a)) {
		c = ";31"
		n = length(a) / 3
		if (n == 1) {
			i = 0
			s = s substr($0, 0, RSTART - 1) "\033[" c "m" a[i]
		} else {
			p = 1
			for (i = 1; i < n; ++i) {
				start = a[i, "start"]
				if (start == null) {
					++n
					continue
				}
				s = s substr($0, p, start - p) "\033[" c "m" a[i]
				p = start + a[i, "length"]
			}
			s = s substr($0, p, RSTART + RLENGTH - p)
		}
		$0 = substr($0, RLENGTH + RSTART) esc_reset
	}
	print s $0
}

