#!/usr/bin/env python3
import re
import sys
from typing import List, Union


sanitize_html_re = re.compile(r"^(<br/>)+|<p>(<br/>|\n)*</p>\n?|(<br/>+|\n)(?=</p>)")
MIN_DESC_CHARS = 80


def is_documented(section_name: str, option_name: str,
                  comment: List[str]) -> Union[bool, None]:
    ignore_parameter = section_name in ("theme", "debug")
    if ignore_parameter:
        return None
    is_enough_documented = sum(map(len, comment)) > MIN_DESC_CHARS
    return is_enough_documented


def htmlize_comment(comments: List[str]) -> str:
    desc = ''.join(comments).replace('<br/><br/>', '</p>\n<p>')
    desc = f'<p>{desc}</p>\n'
    desc = sanitize_html_re.sub('', desc)
    return desc


def to_display_section(s: str) -> str:
    return s.replace('_', ' ')


def advanced_class_group(it) -> bool:
    advanced_counter = sum(it)
    if advanced_counter == 0:
        return 'advanced-False'
    if len(options) == advanced_counter:
        return 'advanced-True'
    return ''


declare_option_re = re.compile(r'^([^ ]+) = ([^(]+)\(.*default="?(.*?)"?\)')
desc_list_re = re.compile(r"^# &nbsp; &nbsp; +([^:]+)(?:: )?(.*)")
nb_desc = 0
nb_params = 0

if len(sys.argv) < 2:
    print(f'{sys.argv[0]} {{file.spec|-}}', file=sys.stderr)
    exit(1)

if sys.argv[1] != '-':
    filename = sys.argv[1]
    f = open(filename)
    filename = filename.rsplit('/', 1)[-1]
    try:
        int(filename)  # check if filename is a fd
        filename = 'rdpproxy.spec'
    except Exception:
        pass
else:
    filename = 'rdpproxy.spec'
    f = sys.stdin

with f:
    html = []
    sections = []
    undocumented = []
    options = []
    section_name = ''
    section_name_displayed = ''

    comments = []
    advanced = False
    display_name = ''
    in_list_elem = False
    special_type = ''

    for line in map(str.rstrip, f):

        # comment
        if line.startswith('#'):
            if line.startswith('# &nbsp; &nbsp;   '):
                if not in_list_elem:
                    in_list_elem = True
                    if comments:
                        comments.append('</p>\n')
                    comments.append('<table>\n')
                item_name, desc = desc_list_re.match(line).groups()
                comments.append(f'<tr><td>{item_name}</td><td>{desc}</td></tr>\n')
            elif line == '#_advanced':
                advanced = True
            elif line.startswith('# (in '):
                if line.startswith('# (in rgb'):
                    special_type = 'RGB color'
                else:
                    special_type = line[6:-1]
            elif line.startswith('#_display_name='):
                display_name = line[15:]
            elif not line.startswith('#_') and not line.startswith('##'):
                if in_list_elem:
                    in_list_elem = False
                    comments.append('</table>\n<p>\n')
                comments.append(line[2:])
                comments.append('<br/>')

        # new section
        elif line.startswith('['):
            section_name = line[1:-1]
            section_name_displayed = to_display_section(section_name)
            options = []
            html = []
            sections.append((section_name, options, section_name_displayed, html))

        # declare option
        elif line:
            # TODO display_name
            option_name, option_type, default_value = declare_option_re.match(line).groups()

            if in_list_elem:
                comments.append('</table>\n')
            desc = htmlize_comment(comments) if comments else ''
            documented_param = is_documented(section_name, option_name,
                                             comments)
            ref = f'{section_name}-{option_name}'

            if documented_param is not None:
                nb_desc += documented_param
                nb_params += 1
                if not documented_param:
                    undocumented.append((f'[{section_name}] {option_name}',
                                         ref, desc, advanced))

            extra = '| advanced' if advanced else ''

            if not display_name:
                display_name = option_name.replace('_', ' ').title()
            options.append((display_name, ref, advanced))

            html.append(f'<div class="option advanced-{advanced}"><h3 id={ref}>'
                        f'[{section_name_displayed}] {display_name}</h3>\n'
                        f'<p>(type: {special_type or option_type}{extra} '
                        f'| default: <code>{default_value}</code>)</p>\n'
                        f'{desc}</div>')

            comments = []
            advanced = False
            display_name = ''
            in_list_elem = False
            special_type = ''


option_name_max_len = 0
nav = ['<nav>']
sections.sort(key=lambda t: t[0])
html_output = []
for section_name, options, section_name_displayed, html in sections:
    advanced_class = advanced_class_group(t[2] for t in options)
    html_output.append(f'<section class="{advanced_class}">\n<h2 id={section_name}>Section: {section_name_displayed}</h2>')
    html_output.append('\n'.join(html))
    html_output.append('</section>')
    nav.append(f'<p class="menu-group {advanced_class}"><a class="menu-item menu-item-section" href=#{section_name}>[{to_display_section(section_name)}]</a>')
    for display_name, ref, advanced in sorted(options, key=lambda t: t[1]):
        nav.append(f'<a class="menu-item advanced-{advanced}" href=#{ref}>{display_name}</a>')
        option_name_max_len = max(option_name_max_len, len(display_name))
    nav.append('</p>')
nav.append('</nav>')

css_column_width = int(option_name_max_len / 1.8)

print(f'''<!DOCTYPE html>\n<html>
<head><title>{filename}</title>
<style>
body {{
    background: #fff;
    margin-left: 0;
    margin-right: 0;
}}
section {{
    border-top: 1px dashed black;
}}

code {{
    background: #eee;
}}

h1, nav {{
    margin-left: .5em;
    margin-right: .5em;
}}

h2 {{
    background: #ddd;
    padding-left: .5em;
    padding-right: .5em;
}}

.option {{
    padding: 1em .5em;
}}

.option:nth-child(even) {{
    background: #f6f6f6;
}}

a:hover, a:active {{
    outline: 2px solid #30AA64;
}}

nav {{
    column-count: auto;
    column-width: {css_column_width}em;
}}

.menu-group {{
    display: inline-grid;
    border: 1px solid black;
    padding: 0;
    width: {css_column_width}em;
}}

.menu-item {{
    display: block;
    padding: .25em .5em;
}}

.menu-item-section {{
    background: #e3e3e3;
}}
</style></head><body>
<h1>{filename}</h1>
<script type="text/javascript">
const head = document.head;
const style = document.createElement("style");
head.appendChild(style);
const setAdvanced = function(a, b) {{
    style.innerText = `
        .advanced-True{{ display: ${{a ? 'block' : 'none'}}; }}
        .advanced-False{{ display: ${{b ? 'block' : 'none'}}; }}
    `;
}}
</script>
<p>Show options:
<button onclick="setAdvanced(true, false)">Advanced</button>
<button onclick="setAdvanced(false, true)">Normal</button>
<button onclick="setAdvanced(true, true)">Both</button>
</p>
''')
print('\n'.join(nav))
print(f"<p>Number of documented parameters = {nb_desc} / {nb_params}</p>")
if undocumented:
    remove_tag = re.compile('<[^>]+>')
    advanced_class = advanced_class_group(t[3] for t in undocumented)
    print(f'<div class="{advanced_class}"><p>Undocumented ({len(undocumented)}):</p><dl id="shortdesc">')
    print(''.join(f'<li class="advanced-{advanced}"><a href="#{ref}">{name}</a>: {remove_tag.sub(" ", desc)}</li>'
                  for name, ref, desc, advanced in undocumented))
    print('</dl></div>')
print('\n'.join(html_output))
print('</section></body></html>')
