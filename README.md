
gen-ftype generates a fragment of C code that translates the
mode of a file to a single-letter mnemonic that is the
same as that printed in the first column of the command:

    ls -l

This messy bit of low-level code is necessary if you want
to do the translation efficiently and still be portable.

Any hand-coded table or hand-coded decoding function
written in any language other than C is not guaranteed to
work, because POSIX says what you can count on, solely
in terms of C code, and even C preprocessor macros.
The position, size and values of the subfield of
`st_mode` containing filetype codes are all not specified.
Only certain macros are claimed to conform to the standard.

In practice, such hand-code methods in some more high-level
language, such as Perl, Python, or Ruby, will likely work.
There is not much variation in these implementation
details.

But, it is still awkward to directly write a translator
for `st_mode` -> ls-filetype, without using C preprocessor
macros.

Question:
  Why not just lift the code from the ls command?
  GNU ls is free software.  
Answer:
  ls is a complicated command.  Both GNU ls and
  Solaris ls are big, complicated, and monolithic.
  Teasing out a simple translation table from that
  code is not all that practical.  It was not designed
  to be a general-purpose, reusable library of small
  functions.  There is no libls.  Not that I know of.
  Not yet, anyway.


# Existing code in Perl

There is perl code in modules on CPAN that uses the
hard-code table.

See, for example,

    http://search.cpan.org/~geotiger/File-Stat-Ls-0.11/Ls.pm
    Module: File::Stat::Ls
    Line: 134

    my @ftype = qw(. p c ? d ? b ? - ? l ? s ? ? ?);

This is the sort of thing that could be improved
by using a C program to generate, at least the
translation table to be plugged in to Perl code,
if not some decoding functions written in C.

# EXAMPLES

Here is a table showing the minor variations in the
generated table, and comparing them to the hand-coded table.

Generated tables:
  Solaris:              q[?pc?d?b?-?l?sDE?]
  Linux:                q[?pc?d?b?-?l?s???]
File::Stat:Ls           q[.pc?d?b?-?l?s???]

# OTHER SYSTEMS

I do not have ready access to HP-UX or AIX systems.
If anyone would care to try out gen-ftype on either
of those, let me know.

I will probably have results from a Free-BSD system,
soon.


TESTING
-------
The code for gen-ftype does not have testing code.
That is, there is no code that is completely
automated, and covers all cases.  Some file types
like Doors and Event Ports are exotic.

I test manually by grovelling through /devices
directory, /tmp and /var/tmp, and in /proc
where some Ds and Ps are likely to be found.

Like so:

  pfexec ls -lh /proc/9/fd/5
  P--------- 2 root root 0 2013-05-29 12:27 /proc/9/fd/5

  pfexec gls -lh /proc/9/fd/5
  ?--------- 2 root root 0 May 29  2013 /proc/9/fd/5

Note that, on Solaris, the native ls command knows
its Ds and Ps, but the 'gls' (GNU version of ls)
does not.

Bigger Picture and Related Work
-------------------------------
gen-ftype is part of a bigger project to make
it easier, not only for C programs, but for
higher-level languages, to extract information
that is "hidden" in the system.  That is, it is
hard to get at in any portable way.

That kind of information is just generally handy,
for many purposes.  The thing I use it for most
is for supplying information about errors:
what file is involved, ls-like information about
the file(s) involved in an error.

There are other kinds of information related to
error reporting that can be improved by using the C
preprocessor to generate decoder tables and/or functions.
For example, a portable strerr_symbol() function
to decode errno values, but to decode to symbols,
such as ENOENT.  It does for symbolic errno values
what strerror() and does for error message strings.
The symbolic translations can be generated completely
automatically, whereas the array of explanatory strings
cannot.  They are maintained by hand.

-- Guy Shaw
   Novice.Sandbox@yahoo.com

