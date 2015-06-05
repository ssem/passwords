from distutils.core import setup

setup(name='recognition',
    version='1.0',
    description='password list parser',
    author='ssem',
    author_email='steven_sem3@hotmail.com',
    scripts=['bin/parse_add_basewords',
             'bin/parse_all',
             'bin/parse_basewords',
             'bin/parse_chars',
             'bin/parse_compexity',
             'bin/parse_entropy',
             'bin/parse_lengths',
             'bin/parse_mods',
             'bin/parse_patterns',
             'bin/parse_prefixes',
             'bin/parse_sets',
             'bin/parse_suffixes',
             'bin/parse_time_and_patterns',
             'bin/parse_top_passwords',
             'bin/parse_years'],
    packages=['rec'],)
