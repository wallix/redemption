#!/usr/bin/awk  -OSf
# 'https://github.com/jonathanpoelen/colout/colout' '-po' '-ER' '^rdpproxy: (INFO [^-]+--  [-=]{3,}>? )([^-=<]*)(.*)?' 'b' 'g' 'b' '--' '-ERci2' '^([a-zA-Z0-9_]+: )?((INFO)|(WARNING)|(ERR)|(NOTICE)|(DEBUG|EMERG|ALERT|CRIT)).* --  (error:)?([a-zA-Z0-9_]+:)?(.*)' 'n' 'n' 'b' 'Y' 'R' 'c' 'W' 'R' '+u' '+ru' '--' '-c' '(Assertion) `(.*)'\'' failed.' 'y' 'R' '--' '^rdpproxy: (\[RDP Session\]) ' 'd' '--' '-cri5' '( type)="([^"]+)"|^([^=]+)="((\\"|[^"])+)"' 'lr' 'lm' 'lb' 'd' '--' '-ER' '^(src/[^:]+|/[^:]+):([:0-9]+) (.)' 'c' 'Y' 'R' '--' '-ER' '^SUMMARY:' 'r'
# gawk profile, created Tue Apr 17 18:18:01 2018

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
	colors2[0] = ";33"
	colors2[1] = ";31;1"
	nb_colors2 = 2
	colors4[0] = ";91"
	colors4[1] = ";95"
	colors4[2] = ";94"
	colors4[3] = ";2"
	nb_colors4 = 4
	colors5[0] = ";36"
	colors5[1] = ";33;1"
	colors5[2] = ";31;1"
	nb_colors5 = 3
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
	}
	if (match($0, /^([a-zA-Z0-9_]+: )?((INFO)|(WARNING)|(ERR)|(NOTICE)|(DEBUG|EMERG|ALERT|CRIT)).* --  (error:)?([a-zA-Z0-9_]+:)?(.*)/, a)) {
		n = length(a) / 3
		if (n == 1) {
			i = 0
			ic = 0
			c = colors1[ic % nb_colors1]
			s = s substr($0, 0, RSTART - 1) "\033[" c "m" a[i] esc_reset
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
				c = colors1[ic % nb_colors1]
				s = s substr($0, p, start - p) "\033[" c "m" a[i]
				p = start + a[i, "length"]
			}
			s = s substr($0, p, RSTART + RLENGTH - p)
		}
		$0 = substr($0, RLENGTH + RSTART) esc_reset
	}
	if (match($0, /(Assertion) `(.*)' failed./, a)) {
		n = length(a) / 3
		if (n == 1) {
			i = 0
			ic = 0
			c = colors2[ic % nb_colors2]
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
				c = colors2[ic % nb_colors2]
				s = s substr($0, p, start - p) "\033[" c "m" a[i] esc_reset
				p = start + a[i, "length"]
			}
			s = s substr($0, p, RSTART + RLENGTH - p)
		}
		$0 = substr($0, RLENGTH + RSTART)
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
				c = colors4[ic % nb_colors4]
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
					c = colors4[ic % nb_colors4]
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
	} else if (match($0, /^(src\/[^:]+|\/[^:]+):([:0-9]+) (.)/, a)) {
		n = length(a) / 3
		if (n == 1) {
			i = 0
			ic = 0
			c = colors5[ic % nb_colors5]
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
				c = colors5[ic % nb_colors5]
				s = s substr($0, p, start - p) "\033[" c "m" a[i]
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
			s = s substr($0, 0, RSTART - 1) "\033[" c "m" a[i] esc_reset
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

