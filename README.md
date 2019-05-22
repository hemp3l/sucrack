![Logo](http://leidecker.info/common/images/sucrack.png)

# sucrack - the su cracker

## About

sucrack is a multithreaded Linux/UNIX tool for brute-force cracking local user accounts via su. This tool comes in handy as final instance on a system where you have not too many privileges but are in the wheel group. 
Many su implementations require a pseudo terminal to be attached in order to take the password from the user. This is why you couldn't just use a simple shell script and pipe the password from STDIN. 
This tool, written in C, is highly efficient and can attempt multiple logins at the same time. Please be advised that using this tool will take a lot of the CPU performance and fill up the logs quite quickly. 

sucrack is so far known to be running on FreeBSD, NetBSD, Linux.
 
 ## Installation

```
./configure
make
make install
```

### additional compile options
       
If you prefer detailed and nice looking statistics to be printed, use the
`--enable-statistics' configuration flag. The downside of that feature is a 
decrease of performance as the statistics have are frequently recalculated. 
If you compiled sucrack without statistics, you still can print a 
minimalistic statistic/progress, that is the number of bytes from the 
wordlist already done and the total number of bytes.

sucrack is able to run multiple threads on su. That actually only makes 
sense, when you are facing a delay for failing authentications. However, 
if you are planning to use multiple threads, compile sucrack with a static 
buffer wordlist (`--with-static-buffer'). This avoids an overhead of a 
dynamic list management. If you are only using one thread it turned out to 
be more efficient to let the dictionary thread put words into a list 
(`--with-dynamic-list') and let the worker thread take one of these, 
whenever it needs to.

It will make sense to link the binary staticly against the libraries. In 
that case, configure sucrack with the --enable-static-linked flag. Default 
is a dynamicly linked sucrack.

See INSTALL for further details.
 
## Run It!

### Options

Before you run sucrack, take a look at the help message or the manpage:

```
sucrack -h
man sucrack
```

In order to run sucrack now, you need to specify a wordlist:

``
sucrack wordlist.txt
````    

Or advise it to read the passwords from stdin. In that case other tools
with smart password generation algorithms could be easily used. For 
instance John The Ripper:

```
john --stdout --incremental | sucrack -
```

You generally will have two options for printing the progress and the
statistics (if you have compiled sucrack with the `--enable-statistics' 
flag). Either by using ansi escapes codes, what makes it look nicer or 
without. The -a flag indicates, whether ansi escape codes should be used or 
not.
   
```
sucrack -a wordlist.txt
````

The interval for reprinting the statistics is set to 3 seconds by default.
You can alter that interval using the -s flag or disable the auto 
reprinting functionality and print the output on any key pressed.

```
sucrack -s 10 -a wordlist.txt
```

This disables the auto reprinting functionality:

```
sucrack -c -a wordlist.txt
```

By default, failed authentications on various Linux distributions causes a 
three seconds delay. sucrack is multithreaded, so that while a thread is 
waiting those seconds, others can do su. It is not advisable to run sucrack 
with more than one worker thread, if there is no such delay, as it slows 
down the overall process.

Run sucrack with ten worker threads:

```
sucrack -w 10 wordlist.txt
```

There is another thread running, besides of the worker threads. The 
dictionary thread reads the words from the wordlist and puts them into
an internal buffer. By default, that buffer is a static array.
You can set the buffer to be a dynamic list with the `--with-dynamic-list'
configuration flag. In both cases, you can alter the size of the buffer 
with the -b option. By default, the buffer size is set to the number of 
worker threads plus one. Consider, that it can't never be less than that.

```
sucrack -b 50 -w 10 wordlist.txt
```

In that example, the dictionary thread will always try to have 50 words
in the buffer to offer them to the 10 worker threads.

If you wan't to su to another user than root, then specify the username
with the -u flag:

```
sucrack -u myuser wordlist.txt
```

The rewriter is a helpful addon. It is rewriting the words from the word
list by certain rules and enqueues them to the word buffer. To enable
the rewriter use -r and to set up your rules -l:

```
sucrack -r -l AFL wordlist.txt
```

Here is an overview over the rules:

```
      rule     description                        original     rewritten
     
      A        all characters to upper case       myPassword   MYPASSWORD
      F        first character to upper case      myPassword   MyPassword
      L        last character to upper case       myPassword   myPassworD
      a        all characters to lower case       AnotherPASS  anotherpass
      f        first character to lower case      AnotherPASS  anotherPASS
      l        last character to lower case       AnotherPASS  AnotherPASs  
      D        prepend a digit (0..9)             password     1password
      d        append a digit (0..9)              password     password1
      e        1337ify the word                   password     p455w0rd
      x        enable all of the above rules
```

All rules run at least once. The `D' and `d' rule rewrite a word ten times 
and append each digit once.

### Environment Variables

sucrack depends on the responses su gives on a failing authentication.
Because that can vary from version to version and distribution to 
distribution you can set the expected responses in environment variables.

```
      environment variable	description

      SUCRACK_SU_PATH		the path to su
      SUCRACK_AUTH_FAILURE	the response of su, if an authentication fails
      SUCRACK_AUTH_SUCCESS      the response sucrack should receive, if an
                                authentication attemp succeeded
```

It is very important to set SUCRACK_AUTH_SUCCESS to any string that can't
be a response of su and does not appear in the wordlist file. Test it, 
before running sucrack:

```
export SUCRACK_AUTH_SUCCESS=banzaii  
grep $SUCRACK_AUTH_SUCCESS wordlist.txt
sucrack wordlist.txt
```

### Troubleshooting & Notice
 
sucrack was tested on Linux, FreeBSD and NetBSD.
