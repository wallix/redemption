#!/usr/bin/awk  -OSf
# https://github.com/jonathanpoelen/colout
# 'colout' '-po' '-ER' '^rdpproxy: (INFO [^-]+--  [-=]{3,}>? )([^-=<]*)(.*)?' 'b' 'g' 'b' '--' '-ER' '^rdpproxy: (INFO.*)?(WARNING.*)?(ERR.*)? --  ([^:]+:)?(.*)' 'b' 'Y' 'R' '+u' '+ru' '--' '^rdpproxy: (\[RDP Session\]) ' 'd' '--' '-cri5' '( type)="([^"]+)"|^([^=]+)="((\\"|[^"])+)"' 'lr' 'lm' 'lb' 'd'
# gawk profile, created Fri Dec 15 12:13:27 2017

# BEGIN rule(s)

BEGIN {
	esc_reset = "\033[0m"
	colors0[0] = ";34"
	colors0[1] = ";32"
	colors0[2] = ";34"
	nb_colors0 = 3
	colors1[0] = ";34"
	colors1[1] = ";33;1"
	colors1[2] = ";31;1"
	colors1[3] = "4"
	colors1[4] = "24"
	nb_colors1 = 5
	colors3[0] = ";91"
	colors3[1] = ";95"
	colors3[2] = ";94"
	colors3[3] = ";2"
	nb_colors3 = 4
}

# Rule(s)

{
	s = ""
	if (match($0, /^rdpproxy: (INFO [^-]+--  [-=]{3,}>? )([^-=<]*)(.*)?/, a)) {
		n = length(a) / 3
		if (n == 1) {
			i = 0
			ic = 0
			c = colors0[ic % nb_colors0]
			s = s substr($0, 0, RSTART - 1) "\033[" c "m" a[i] esc_reset
		} else {
			p = 1
			for (i = 1; i < n; ++i) {
				start = a[i, "start"]
				if (start == null) {
					++n
					continue
				}
				ic = i - 1
				c = colors0[ic % nb_colors0]
				s = s substr($0, p, start - p) "\033[" c "m" a[i]
				p = start + a[i, "length"]
			}
			s = s substr($0, p, RSTART + RLENGTH - p)
		}
		$0 = substr($0, RLENGTH + RSTART) esc_reset
	} else if (match($0, /^rdpproxy: (INFO.*)?(WARNING.*)?(ERR.*)? --  ([^:]+:)?(.*)/, a)) {
		n = length(a) / 3
		if (n == 1) {
			i = 0
			ic = 0
			c = colors1[ic % nb_colors1]
			s = s substr($0, 0, RSTART - 1) "\033[" c "m" a[i] esc_reset
		} else {
			p = 1
			for (i = 1; i < n; ++i) {
				start = a[i, "start"]
				if (start == null) {
					++n
					continue
				}
				ic = i - 1
				c = colors1[ic % nb_colors1]
				s = s substr($0, p, start - p) "\033[" c "m" a[i]
				p = start + a[i, "length"]
			}
			s = s substr($0, p, RSTART + RLENGTH - p)
		}
		$0 = substr($0, RLENGTH + RSTART) esc_reset
	} else if (match($0, /^rdpproxy: (\[RDP Session\]) /, a)) {
		c = ";2"
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
				c = colors3[ic % nb_colors3]
				s = s substr($0, 0, RSTART - 1) "\033[" c "m" a[i] esc_reset
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
					c = colors3[ic % nb_colors3]
					s = s substr($0, p, start - p) "\033[" c "m" a[i] esc_reset
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
	print s $0
}

