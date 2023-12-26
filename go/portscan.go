/* ----------------------------------------------------------------------
--
-- portscan
--
-- Takes a file containing lines of hostname:port and tries to connect
-- to each using TCP.  Uses concurrancy to speed up the operation.
--
-- Based on getaddrinfo so supports ipv4 & ipv6 with DNS and service
-- name resolution.
--
-- 2018-08-21: Ross Alexander
--    Add comment regexp feature
--    Add -timeout=<string>
--    Add support for multiple input files
--    Extensive commenting added
--
-- 2018-07-24: Ross Alexander
--
---------------------------------------------------------------------- */

package main

import ("fmt"
        "strings"
        "io/ioutil"
        "os"
        "net"
        "time"
        "sync"
	"flag"
	"regexp"
)

type portscan_t struct {
	timeout string
}

/* ----------------------------------------------------------------------
--
-- conn_check
--
---------------------------------------------------------------------- */

func conn_check(opts portscan_t, host string, wg *sync.WaitGroup) {

/* --------------------
Convert into an int64 nanosecond count
-------------------- */

	dur, err := time.ParseDuration(opts.timeout)

/* --------------------
Use net.DailTimeout to make outbound TCP connection
-------------------- */

	conn, err := net.DialTimeout("tcp", host, dur)

/* --------------------
Check if error and complain if so, otherwise report success and close the connection
-------------------- */
	
        if err != nil {
                fmt.Printf("Connection to %s failed: %s\n", host, err)
        } else {
                fmt.Printf("Connection to %s succeded\n", host)
                conn.Close();
        }

/* --------------------
Let the waitgroup know this thread is done
-------------------- */
	
        wg.Done()
}

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

func main() {

/* --------------------
Declare global variable structure
-------------------- */
	
 	var opts portscan_t

/* --------------------
-- Process command line options
-------------------- */
	
	flag.StringVar(&opts.timeout, "timeout", "2s", "time it will wait for the connection to open");
	flag.Parse()

/* --------------------
Remaining command line parameter should be a file, otherwise exit
-------------------- */
	
	if len(flag.Args()) < 1 {
                fmt.Println("portscan <file>")
                os.Exit(1)
        }

/* --------------------
Loop over remaining options (minimum one entry)
-------------------- */
	
	for _, path := range flag.Args() {

/* --------------------
Read file into string to die
-------------------- */

		content, err := ioutil.ReadFile(path)
		if err != nil {
			fmt.Printf("Could not open file %s\n", path)
			os.Exit(1)
		}

/* --------------------
Split the file by newline
-------------------- */

		lines := strings.Split(string(content), "\n")

/* --------------------
Use sync.WaitGroup to multithead conn_check
-------------------- */

		var waitGroup sync.WaitGroup

/* --------------------
Loop over each line of the file
-------------------- */

		for _, v := range lines {

/* --------------------
Use regexp to strip any comments (ie text after a #)
-------------------- */
			
			re := regexp.MustCompile("[\t\n\f\r ]*#.*$")
			v = re.ReplaceAllString(v, "")

/* --------------------
Ignore blank lines
-------------------- */

			if (len(v) > 0) {

/* --------------------
Incrase the semaphore on the waitgroup
-------------------- */

				waitGroup.Add(1)

/* --------------------
Use go's magic go threaded function call
-------------------- */
				
				go conn_check(opts, v, &waitGroup)
			}
		}

/* --------------------
Wait for all threads to exit
-------------------- */

		waitGroup.Wait()
	}
}
