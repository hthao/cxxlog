# cxxlog
A simple c++ class for logging. 

+ Efficient
+ Thread safe
+ Easy to use

##Format
`timestamp` `[thread ID]` `<log level>` `log content`
##Example

####`code`
```c

CXXLOG_OPEN("./test.log", true); /* "true" is meant to output to stdout. */
CXXLOG_TRACE("this is a cxxlog example, %s, %d.\n", "hello world", 10);
CXXLOG_DEBUG("this is a cxxlog example, %s, %d.\n", "hello world", 10);
CXXLOG_INFO("this is a cxxlog example, %s, %d.\n", "hello world", 10);
CXXLOG_WARN("this is a cxxlog example, %s, %d.\n", "hello world", 10);
CXXLOG_ERROR("this is a cxxlog example, %s, %d.\n", "hello world", 10);

```
####`output`

```
20160406 14:03:37.911 [30210] <TRACE> this is a cxxlog example, hello world, 10.
20160406 14:03:37.911 [30210] <DEBUG> this is a cxxlog example, hello world, 10.
20160406 14:03:37.911 [30210] <INFO> this is a cxxlog example, hello world, 10.
20160406 14:03:37.911 [30210] <WARNING> this is a cxxlog example, hello world, 10.
20160406 14:03:37.911 [30210] <ERROR> this is a cxxlog example, hello world, 10.
```
##Dependency
* c++11
* libpthread
