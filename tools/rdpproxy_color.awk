#!/usr/bin/awk  -OSf
# colout '-po' '-ER' '^rdpproxy: (INFO [^-]+-- [-=]{3,}>? )([^-=<]*)(.*)?' 'b' 'g' 'b' '--' '-ERci2' '^([a-zA-Z0-9_]+: )?((INFO)|(WARNING)|(ERR)|(NOTICE)|(DEBUG|EMERG|ALERT|CRIT)).* -- (error:)?([a-zA-Z0-9_]+:)?(.*)' 'n' 'n' 'b' 'Y' 'R' 'c' 'W' 'R' '+u' '+ru' '--' '-c' '(src/[^:]+|/[^:]+):([:0-9]+)' 'c' 'Y' '--' '-c' '(Assertion) `(.*)'\'' failed.' 'e7,o' 'R' '--' '^rdpproxy: (\[RDP )(Session|Proxy\]) ' '137' '--' '-cri5' '( type)="([^"]+)"|^([^=]+)="((\\"|[^"])+)"' 'lr' 'lm' 'lb' '243' '--' '-ER' '^(src/[^:]+|/[^:]+):([:0-9]+) ([^:]+)(.?)' 'c' 'Y' 'R' 'o' '--' '-ER' '^SUMMARY:' 'r' '--' '^    (#[0-9]+) [^ ]+ in (.+ )([^ :]+)(:[0-9]+)(:[0-9]+)?' 'r' 'm' 'c' 'Y' 'Y' '--' '^    (#[0-9]+) [^ ]+ in (.*) \(([^+]+)\+0x' 'r' 'm' 'n'
# gawk profile, created Tue Jan 15 14:24:44 2019

# BEGIN rule(s)

BEGIN {
	esc_reset = "\033[0m"
	colors0[0] = ";34"
	colors0[1] = ";32"
	colors0[2] = ";34"
	nb_colors0 = 3
	colors1[0] = ";0"
	colors1[1] = ";0"
	colors1[2] = ";34"
	colors1[3] = ";33;1"
	colors1[4] = ";31;1"
	colors1[5] = ";36"
	colors1[6] = ";97;1"
	colors1[7] = ";31;1"
	colors1[8] = "4"
	colors1[9] = "24"
	nb_colors1 = 10
	colors2[0] = ";36"
	colors2[1] = ";33;1"
	nb_colors2 = 2
	colors3[0] = ";37;1"
	colors3[1] = ";31;1"
	nb_colors3 = 2
	colors5[0] = ";91"
	colors5[1] = ";95"
	colors5[2] = ";94"
	colors5[3] = ";38;5;243"
	nb_colors5 = 4
	colors6[0] = ";36"
	colors6[1] = ";33;1"
	colors6[2] = ";31;1"
	colors6[3] = ";1"
	nb_colors6 = 4
	colors8[0] = ";31"
	colors8[1] = ";35"
	colors8[2] = ";36"
	colors8[3] = ";33;1"
	colors8[4] = ";33;1"
	nb_colors8 = 5
	colors9[0] = ";31"
	colors9[1] = ";35"
	colors9[2] = ";0"
	nb_colors9 = 3
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
	if (match($0, /^([a-zA-Z0-9_]+: )?((INFO)|(WARNING)|(ERR)|(NOTICE)|(DEBUG|EMERG|ALERT|CRIT)).* -- (error:)?([a-zA-Z0-9_]+:)?(.*)/, a)) {
		n = length(a) / 3
		if (n == 1) {
			i = 0
			ic = 0
			s = s substr($0, 0, RSTART - 1) "\033[" colors1[ic % nb_colors1] "m" a[i]
		} else {
			p = 1
			for (i = 1; i < n; ++i) {
				if (i == 2) {
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
	if (match($0, /(src\/[^:]+|\/[^:]+):([:0-9]+)/, a)) {
		n = length(a) / 3
		if (n == 1) {
			i = 0
			ic = 0
			s = s substr($0, 0, RSTART - 1) "\033[" colors2[ic % nb_colors2] "m" a[i] esc_reset
		} else {
			p = 1
			for (i = 1; i < n; ++i) {
				start = a[i, "start"]
				if (start == null) {
					++n
					continue
				}
				ic = i - 1
				s = s substr($0, p, start - p) "\033[" colors2[ic % nb_colors2] "m" a[i] esc_reset
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
	} else if (match($0, /^rdpproxy: (\[RDP )(Session|Proxy\]) /, a)) {
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
				s = s substr($0, 0, RSTART - 1) "\033[" colors5[ic % nb_colors5] "m" a[i] esc_reset
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
					s = s substr($0, p, start - p) "\033[" colors5[ic % nb_colors5] "m" a[i] esc_reset
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
	} else if (match($0, /^(src\/[^:]+|\/[^:]+):([:0-9]+) ([^:]+)(.?)/, a)) {
		n = length(a) / 3
		if (n == 1) {
			i = 0
			ic = 0
			s = s substr($0, 0, RSTART - 1) "\033[" colors6[ic % nb_colors6] "m" a[i]
		} else {
			p = 1
			for (i = 1; i < n; ++i) {
				start = a[i, "start"]
				if (start == null) {
					++n
					continue
				}
				ic = i - 1
				s = s substr($0, p, start - p) "\033[" colors6[ic % nb_colors6] "m" a[i]
				p = start + a[i, "length"]
			}
			s = s substr($0, p, RSTART + RLENGTH - p)
		}
		$0 = substr($0, RLENGTH + RSTART) esc_reset
	} else if (match($0, /^SUMMARY:/, a)) {
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
	} else if (match($0, /^    (#[0-9]+) [^ ]+ in (.+ )([^ :]+)(:[0-9]+)(:[0-9]+)?/, a)) {
		n = length(a) / 3
		if (n == 1) {
			i = 0
			ic = 0
			s = s substr($0, 0, RSTART - 1) "\033[" colors8[ic % nb_colors8] "m" a[i] esc_reset
		} else {
			p = 1
			for (i = 1; i < n; ++i) {
				start = a[i, "start"]
				if (start == null) {
					++n
					continue
				}
				ic = i - 1
				s = s substr($0, p, start - p) "\033[" colors8[ic % nb_colors8] "m" a[i] esc_reset
				p = start + a[i, "length"]
			}
			s = s substr($0, p, RSTART + RLENGTH - p)
		}
		$0 = substr($0, RLENGTH + RSTART)
	} else if (match($0, /^    (#[0-9]+) [^ ]+ in (.*) \(([^+]+)\+0x/, a)) {
		n = length(a) / 3
		if (n == 1) {
			i = 0
			ic = 0
			s = s substr($0, 0, RSTART - 1) "\033[" colors9[ic % nb_colors9] "m" a[i] esc_reset
		} else {
			p = 1
			for (i = 1; i < n; ++i) {
				start = a[i, "start"]
				if (start == null) {
					++n
					continue
				}
				ic = i - 1
				s = s substr($0, p, start - p) "\033[" colors9[ic % nb_colors9] "m" a[i] esc_reset
				p = start + a[i, "length"]
			}
			s = s substr($0, p, RSTART + RLENGTH - p)
		}
		$0 = substr($0, RLENGTH + RSTART)
	}
	print s $0
}

