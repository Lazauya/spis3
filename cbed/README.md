This is CBED (Custom BaseExtraData)
An explanation:
there are some unused slots in the BaseExtraList type bitfield, so we can hack stuff to include our own extradata
However, there are only a maximum of 4 slots. It could be enlarged, but that would be a lot of work. So for now,
it's only 4. What you can do instead is implement a single etra type and give it multiple attributes.

How to use:
make a class deriving the BaseExtraData and override all virtual functions. Your destructor is a bit trickier; from
my own tests, its best if you use the destructor that is provided to you by the BaseExtraData. You do this by 
overwriting the VTable of your custom type. You need to get the vtable and overwrite the first entry. See an example
in SPIS.