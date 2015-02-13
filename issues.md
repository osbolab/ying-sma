#9 Specify JNI interface for Android client
> tags: android, jni  
> opened: 12/8/14

The Android client needs to be able to

  * Create Internet and Bluetooth links
  * Create the link layer with links
  * Create a forwarding strategy
  * Add forwarding strategy to link layer
  * Create a node ID
  * Create a context with node ID and link layer
  * Create components, e.g. GPS
  * Add components to context
  * Create a node with node ID and context
  * Create CCN helpers with node
  * Connect link layer to node
  * Stop node
  * Stop link layer
  * Destroy both

Most of this can be done in C++ and exposed to Android via a simplified interface.

The Android client also needs to define the `Async` members and implement the `Link` interface.


#~~8 Add Android project to SMA tree~~
> tags: android, core  
> opened: 12/8/14  
> closed: 12/8/14


#~~7 Should the outgoing message buffer be unbounded, overwriting, or strictly bounded?~~
> tags: core, linklayer  
> opened: 12/8/14  
> closed: 12/8/14

Writing datagrams to the network should block only as long as the OS's outgoing buffer is full,
which should be rare and clear quickly. An overrun outgoing message buffer indicates that

  * The node's average message production rate exceeds the link rate
  * The link is in an invalid state and is not accepting datagrams
  * The thread writing to the link is deadlocked

Thus the current outgoing buffer implementation considers overruns exceptional as all of these are
bugs. I think this is the correct implementation; I'm just opening this issue to remind myself
to keep it in mind.


#6 Add between-neighbor block transfer
> tags: ccn, blocks  
> opened: 12/8/14

The first step to multihop block (and content) fetching is reliable between-neighbor block 
dissemination. Since the underlying protocols are not inherently reliable successfully
transferring blocks above a trivially small size requires some additional deconstruction at the
source and reconstruction at the destination.

Since a block request arrives at the producer along a single path the resulting block should
be sent along a single path. In a mobile network, however, the probability that this path will be
interrupted is high enough that some redundancy is mandatory for reasonable average-case 
performance.

The first redundancy scheme to be implemented is:

  1. Let the content producer be the transferring node.
  1. The transferring node segments the block to fit the link's MTU and begins broadcasting the 
  following segment message:
    
           ___________________________
          |  Block ID  |  Block size  |
          |---------------------------|
          | Segment #  | Segment size |
          |---------------------------|
          |   Ordered recipient set   |
          |---------------------------|
          |        Segment data       |
           ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾

  1. When a node receives such a segment it will go silent until all segments are received or
  some timeout is elapsed. Each received segment resets the timeout. The node allocates the block
  in its cache and copies the segment into it, marking that segment *received*.
  1. Once the transferring node has transferred all segments it will broadcast a *completed*
  message. When this message is received, each node listed in the *ordered recipient set* may, in
  turn, broadcast any segments of the block it is missing. The transferring node will aggregate
  these segments and, when all nodes have had their chance, will rebroadcast those segments.
  This process repeats until no nodes report missing segments.
  1. Any node that has an entry in its *Pending Request Table* for that block will become the
  transferring node and repeat the process from step 2.


#5 Complete block request messages
> tags: ccn, blocks  
> opened: 12/8/14

A block request should be sufficient to identify a particular block of a particular item of
content optionally at a particular node.

On receiving a block request a node should store that block ID in its *Pending Request Table* 
(PRT). If a node receives a block that it finds in its PRT it will rebroadcast that block.


#4 Codify block naming
> tags: ccn, blocks  
> opened: 12/8/14

Presently there is no standard way that chunks are named so that they can be requested by
consumers.

Such a name should be sufficient to identify a particular block of a particular item of content on
whichever node is providing that content.


#3 Use 'dirty neighbor' lists limit interest/content rebroadcasting.
> tags: ccn, interests, metadata  
> opened: 12/8/14

Interest and content announcements are rebroadcast whenever they are received and then 
periodically until they expire. Instead they should only be rebroadcast by a node when it has
some neighbor that has not demonstrated that it has those records.

Each node should maintain for each interest/content record a set of its neighbors from which it
has not seen that record. It will include the record in its rebroadcasts until that set is empty.
On seeing a record broadcast by a neighbor it removes that neighbor from the set for that record.
When a new neighbor arrives it is added to the sets of all records held by the node.


#~~2 Use memcpy instead of stringbuf to serialize to/from byte arrays.~~
 > tags: core, serialization  
 > opened: 12/8/14  
 > closed: 12/8/14  

`BinaryInput` and `BinaryOutput` use `BinarySource`/`BinaryDest` to wrap a byte array in an 
`iostream` using `std::stringbuf::pubsetbuf`. This is totally unnecessary because the only members
 of iostream that they use are `read(char*, size_t)` and `write(char const*, size_t)`.

 Remove `BinarySource` and `BinaryDest` completely and replace the `iostream` dependencies with
 `std::memcpy`.


#1 Establish a standard document
> tags: documentation  
> opened: 12/8/14

There is presently no document describing the protocols or data structures in any standard way.
Such a document should be sufficient to implement a node that can transparently interact with the
network in any language on any platform; therefore it should specify the network protocols and
standard client behavior that produces the overall network behavior.