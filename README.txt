
Όνομα:Λήδα Ζαχαροπούλου
Α.Μ, 1115201100004


================================ ΤΡΟΠΟΣ ΥΛΟΠΟΙΗΣΗΣ ===========================================

Η σειρά των options -k, -h στο τέλος,μπορεί να είναι οποιαδήποτε,
με την προυπόθεση ότι αν έχει δωθεί PORT2, αυτό θα βρίσκεται ακριβώς μετά το ADDRESS ([-k NUM] [-h ADDRESS [PORT2] ])
Η διαχείρηση των ορισμάτων γίνεται από τη συνάρτηση manageArguments στο αρχείο main.c

Ο αριθμός των sections που θα έχει το bloom filter βρίσκεται ως εξής:Οσο τα sections > size/sections,αυξάνεται το i
τα section_bytes γίνονται i*MIN_SECTION_BYTES,(όπου MIN_SECTION_BYTES 64) και ο αριθμός των sections γίνεται size/section_bytes.
Αν το size αφήνει υπόλοιπο,ορίζεται και μια τελευταία επιπλέον περιοχή.Έτσι αποφεύγεται
Coarse-grained και Fine-grained locking όταν κλειδόνονται τα mutexes των sections

H oracle αρχικοποιείται με seed από τον χρήστη.Για κάθε επανάληψη νήματος και για κάθε νήμα
δημιουγείται μια τυχαία λέξη με κεφαλαία, μήκους MAX_WORD_LENGTH με τη συνάρτηση wordGenerator στο
functions.c

H main δημιουργεί τα thread αναζήτησης και στη συνέχεια ένα socket,όπου μπλοκάρει στην accept περιμένωντας συνδέσεις.
To socket αυτό παραμένει ανοιχτό μέχρι να επιστρέψει και το τελευταίο thread αναζήτησης,το οποίο το κάνει shutdown
και έτσι ξεμπλοκάρει.Αυτό γίνεται με τη χρήση μετρηρή των threads που έχουν ήδη τελειώσει.

Η αναζήτηση γίνεται με την συνάρτηση search στο αρχείο search.c
Oι λέξεις που επιστρέφει η oracle τοποθετούνται σε μια στοίβα προς εξέταση.Η στοίβα δεν δεσμεύει καινούργιο
χώρο για την λέξη αλλά αποθηκεύει δείκτη προς το αποτέλεσμα της οracle (results[i]) ενώ πριν από κάθε νέα
κλήση απελευθερώνεται και ο χώρος του πίνακα (results).Σε κάθε επανάληψη της search, γίνεται pop η πρώτη
λέξη από τη στοίβα, γίνεται έλεγχος για το αν βρίσκεται ήδη στο bloom filter και αν όχι,καλέιται η oracle για αυτην.
Σε κάθε πέρίπτωση στο τέλος η λέξη αποδεσμεύεται.

Το γράψιμο στο bloomfilter γίνεται μέσω buffer μεγέθους BUF_SIZE (4096).Πρώτα αντιγράφεται από το thread,ώστε
να μην κρατάει όλα τα mutexes του bloom κλειδωμένα για πολύ ώρα,στη συνέχεια στέλνεται σε τμήματα
και μετά ξανα-αποδεσμεύεται.


=================================== ΠΑΡΑΔΕΙΓΜΑΤΑ ΕΚΤΕΛΕΣΗΣ =========================================================

H μνήμη που χρησιμοποιείται σε κάθε περίπτωση σταθεροποιείται στο 9.6% του Mem: 1025208k total ,δηλαδή ~98.419.968 MB

******* 1 thread VS 2 thread ***********

Bloom 100MB, Loops 5, Seed 10, No mode


1 thread:

./invoke-oracle-multithreaded 10000000 1 5 666666 stats0

Searching word...
Size:10000000 N:1 L:5 Port:666666 Logfile:stats0 k:3
Please give an integer as seed:10
Bloom Filter will be divided in 3125 section(s) of 3200 bytes

Server: Create socket to port:666666
Server: Listening port 666666 for connections via socket with fd 4
Thread 3068185408 searching for word YBHZDKOZNVSA
Thread 3068185408,Loop 0:Word not Found!
Thread 3068185408 searching for word UZTWDVYBQWWC
Thread 3068185408,Loop 1:Word not Found!
Thread 3068185408 searching for word ESDCJPPUEYWF
Thread 3068185408,Loop 2:Word not Found!
Thread 3068185408 searching for word GQLHPAFGFZMJ
Thread 3068185408,Loop 3:Word not Found!
Thread 3068185408 searching for word ZUVIDOHNUFUS
Thread 3068185408,Loop 4:Word not Found!
Deleting empty stack
Last thread 3068185408 closing socket with fd 4
Program execute time 24.540000

2 threads:
./invoke-oracle-multithreaded 10000000 2 5 666666 stats0

Searching word...
Size:10000000 N:2 L:5 Port:666666 Logfile:stats0 k:3
Please give an integer as seed:10
Bloom Filter will be divided in 3125 section(s) of 3200 bytes

Server: Create socket to port:666666
Server: Listening port 666666 for connections via socket with fd 4
Thread 3060136768 searching for word PDXJRRTWYLBF
Thread 3060136768,Loop 0:Word not Found!
Thread 3060136768 searching for word BXHPHBVZMRFW
Thread 3068529472 searching for word UVEFYHLHNHAE
Thread 3068529472,Loop 0:Word not Found!
Thread 3068529472 searching for word QVOWKRBTVYQI
Thread 3060136768,Loop 1:Word not Found!
Thread 3060136768 searching for word QWHUHVIGFLTW
Thread 3060136768,Loop 2:Word not Found!
Thread 3060136768 searching for word YQRVJGSSNBKZ
Thread 3060136768,Loop 3:Word not Found!
Thread 3060136768 searching for word OVBWBUMXCESR
Thread 3060136768,Loop 4:Word not Found!
Deleting empty stack
Thread 3068529472,Loop 1:Word not Found!
Thread 3068529472 searching for word BZCHCZBDEUIZ
Thread 3068529472,Loop 2:Word not Found!
Thread 3068529472 searching for word XVCIAYVVNGYF
Thread 3068529472,Loop 3:Word not Found!
Thread 3068529472 searching for word CPSJGYLIIZIW
Thread 3068529472,Loop 4:Word not Found!
Deleting empty stack
Last thread 3068529472 closing socket with fd 4
Program execute time 22.720000


********* Ενδεικτικές εκτυπώσεις server-client 4 διεργασιών, στο ίδιο μηχάνημα, seed 8 *************

Η διεργασία 1 λειτουργεί σαν server για τις 2 και 3, ενώ η 2 γίνεται με τη σειρά της server για την 4.

> Διεργασία 1 ./invoke-oracle-multithreaded 100000000 2 5 12345 stats3

Server: Create socket to port 12345
Server: Listening port 12345 for connections via socket with fd 4
Thread 2969185088 searching for word FFRTEJCEOMGS
Thread 2977577792 searching for word UFQZCKKXQYBK
Server: Accepted connection to new socket with fd 5
Server: Spawn a new thread to handle the communication with the client
Thread 2956983104 connecting to socket with fd 5
 Writing arguments to socket
 Send all to socket
 Client, are all arguments sent correct? (y/n) y
 OK,I will send the bloom filter!
 Total bytes of bloom filter written 100001814
Thread 2956983104 is closing socket with fd:5
Server: Client thread 2956983104 joined
Server : Will close socket with fd 5
Server: Accepted connection to new socket with fd 5
Server: Spawn a new thread to handle the communication with the client
Thread 2956983104 connecting to socket with fd:5
 Writing arguments to socket
 Send all to socket
 Client, are all arguments sent correct? (y/n) y
 OK,I will send the bloom filter!
 Total bytes of bloom filter written 100001814
Thread 2956983104 is closing socket with fd:5
Server: Client thread 2956983104 joined
Server : Will close socket with fd 5
Thread 2969185088,Loop 0:Word not Found!
[...]
Last thread 2977577792 closing sock with fd 4
Program execute time 226.100000

> Διεργασία 2  ./invoke-oracle-multithreaded 100000000 2 5 12345 stats2 -h localhost 666666

Client: Connected to localhost, port 12345 via socket with fd 3
Client: Received size 100000000 threadsNum 2 loops 5 port 12345 logfile stats3 k 3 seed 8
Client: All arguments correct!
Client: Will copy bloomfilter!
Client: Total bytes of bloom filter read 100001814
Client: Closing socket with fd 3
Bloom Filter will be divided in 9952 section(s) of 10048 bytes
plus 1 of 2304 bytes

Server: Create socket to port 666666
Server: Listening port 666666 for connections via socket with fd 4
Thread 2966203200 searching for word ECETBBJLIQWO
Thread 2974595904 searching for word TYMZNGZPOCRR
Thread 2974595904,Loop 0:Word not Found!
Thread 2974595904 searching for word JSZETHLGXFRU
Server: Accepted connection to new socket with fd 5
Server: Spawn a new thread to handle the communication with the client
Thread 2953837376 connecting to socket with fd:5
 Writing arguments to socket
 Send all to socket
 Client, are all arguments sent correct? (y/n) y
 OK,I will send the bloom filter!
 Total bytes of bloom filter written 100001814
Thread 2953837376 is closing socket with fd:5
Server: Client thread 2953837376 joined
Server : Will close socket with fd 5
Thread 2966203200,Loop 0:Word not Found!
[...]
Last thread 2966203200 closing sock with fd 4
Program execute time 270.350000


Διεργασία 3 ./invoke-oracle-multithreaded 100000000 2 5 12345 stats3 -h localhost 77777

Client: Connected to localhost, port 12345 via socket with fd 3
Client: Received size 100000000 threadsNum 2 loops 5 port 12345 logfile stats3 k 3 seed 8
Client: All arguments correct!
Client: Will copy bloomfilter!
Client: Total bytes of bloom filter read 100001814
Client: Closing socket with fd 3

Server: Create socket to port 77777
Server: Listening port 77777 for connections via socket with fd 4
Thread 2966764352 searching for word OOJXJKFBMOUM
Thread 2975157056 searching for word NCIDWBLASUKO
[...]
Last thread 2966764352 closing sock with fd 4
Program execute time 211.240000


Διεργασία 4 ./invoke-oracle-multithreaded 100000000 2 5 666666 stats4 -h localhost 34345

Client: Connected to localhost, port 666666 via socket with fd 3
Client: Received size 100000000 threadsNum 2 loops 5 port 666666 logfile stats2 k 3 seed 8
Client: All arguments correct!
Client: Will copy bloomfilter!
Client: Total bytes of bloom filter read 100001814
Client: Closing socket with fd 3

Server: Create socket to port 34345
Server: Listening port 34345 for connections via socket with fd 4
Thread 2966784832 searching for word KKVUVTEYFIOZ
[....]
Last thread 2966784832 closing sock with fd 4
Program execute time 181.320000


********************** 5 word length vs 12 word length***************************

-Για 100ΜB Bloom, 1 thread, 5 loops,k 3 seed 9 (no mode)

LENGTH 5:

Server: Create socket to port 12345
Server: Listening port 12345 for connections via socket with fd 4
Thread 2977717056 searching for word LUIKF
Thread 2977717056,Loop 0:Word not Found!
Thread 2977717056 searching for word ROOBH
Thread 2977717056,Loop 1:Word not Found!
Thread 2977717056 searching for word HNAER
Thread 2977717056,Loop 2:Word not Found!
Thread 2977717056 searching for word QPAJR
Thread 2977717056,Loop 3:Word not Found!
Thread 2977717056 searching for word IJIGH
Thread 2977717056,Loop 4:Word not Found!
Deleting empty stack
Last thread 2977717056 closing sock with fd 4
Program execute time 283.050000

LENGTH 12:

Server: Create socket to port 12345
Server: Listening port 12345 for connections via socket with fd 4
Thread 2977340224 searching for word WCBMGXJBZXXL
Thread 2977340224,Loop 0:Word not Found!
Thread 2977340224 searching for word EZNNMJXJIJOD
Thread 2977340224,Loop 1:Word not Found!
Thread 2977340224 searching for word UNXQMUDASVYG
Thread 2977340224,Loop 2:Word not Found!
Thread 2977340224 searching for word OLFSGSKSXMMJ
Thread 2977340224,Loop 3:Word not Found!
Thread 2977340224 searching for word PGPXJZWUQCCR
Thread 2977340224,Loop 4:Word not Found!
Deleting empty stack
Last thread 2977340224 closing socket with fd 4
Program execute time 36.530000

************************ -02 Optimization ****************************

1)Για ./invoke-oracle-multithreaded 100000000 1 5 12345 stats2, seed 8, EasyMode

Ενδεικτικοί χρόνοι στις περιπτώσεις που δεν βρίσκει λύση (ocetznchjrfrirjwmehplqjymzuhx)

Με -02 :

(6/10 φορές δεν βρήκε λύση)

Program execute time 1.470000
Program execute time 3.510000
Program execute time 5.380000
Program execute time 5.180000
Program execute time 6.660000
Program execute time 0.580000

Xωρίς -Ο2
(6/10 φορές δεν βρήκε λύση)

Program execute time 2.170000
Program execute time 15.000000
Program execute time 21.900000
Program execute time 9.950000
Program execute time 6.490000
Program execute time 11.750000
--------------------------------------------

2) Για 100ΜB Bloom, 2 threads, 5 loops, seed 10 (no mode)***

Στην περίπτωση που δεν βρίσκει λύση,οι αντίστοιχοι 
χρόνοι σε secs είναι:

Με -Ο2
Program execute time 209.820000 

Χωρίς -Ο2
Program execute time 305.120000
----------------------------------------------

Γενικά παρατηρούμε ότι οι διαφορές με το optimization δεν είναι πολύ μεγάλες
Όταν όμως αρχικά,είχα κάνει υλοποίηση με αναδρομή,οι διαφορές ήταν τεράστιες,
σε σημέιο που το πρόγραμμα σχεδόν δεν δούλευε αν δεν είχει το -Ο2

=======================================================================================================
