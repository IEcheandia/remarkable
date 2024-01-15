# The LUA interpreter filter (LUA v5.3)

The LUA interpreter filter uses standard LUA syntax to read and write filter pin values. There are three supported types, [Scalar](#scalars), [Line](#lines) and [Image](#images). The filter allows access not only to data values but also rank and result information.

LUA filters are meant for quick prototyping but are too inefficient for usage in realtime applications.

## Terminology

| Term | Description |
| ---- | ---- |
| Context | Mandatory property of all pin values, used for output analysis. Generally all inputs/outputs should have the same context. |
| Score | Signal reliability for the whole output in ```[0..1]``` (float values). Denotes the quality of the output result. |
| Rank | Signal reliability for each individual element of an output value in ```[0..255]``` (integer values). Same as Score but for subvalues. |
| Result | Analysis flag. Usually set to "AnalysisOk" but can carry additional information about what the output values respresent. |
| Scalar | Float values |
| Line | Fixed-size list of float values |
| Image | Raster image storing grayscale pixels in bytes |

## Getting started

## Custom functions:

The default print statement has been forwarded to the Weldmaster output log (info category). Multiple arguments get concatenated and indented.
> print ( s, ... )

The ```fail``` function allows for error handling in LUA. It prints an error string to the Weldmaster output log (error category) and stops LUA execution for all future frames. Multiple arguments get concatenated and indented, same as ```print```.
> fail ( e, ... )

All filters have the ability to keep state over multiple frames. Similarly, the ```process``` function allows the user to define persistent, global variables in their LUA script. If the ```process``` function is present, the LUA script will only be executed once per analysis and the ```process``` function is called each frame.

```
-- Persistent variables
X = 0

-- Executed each frame
function process ()
	X = X + 1
	print(X)
end
```

If there is no ```process``` function the whole script will be executed each frame,
but one can get the same outcome using [_N](#_n):

```
-- Initialize X
if _N == 1 then
	X = 0
end

-- X is now valid
X = X + 1
print(X)
```

Whether output values (e.g. sR) remain persistent can be changed in the settings.
Either way they are reset to 0 for the first frame and are safe to access.

## Scalars

Scalars represent value pins, but since we support multiple values per pin they act like a list of numbers.

Input and output pins are:

> sA sB \
> sR sS

Score and result of the whole scalar structure in ```[0 .. 1]``` can be read and written with the ```score``` and ```result``` functions:

```
s = sA:score()
sA:score( 0.1 )
r = sA:result()
sA:result( Results.AnalysisOk )
```

For possible result and score values, see [_Results constants](#Constants).

The elements can be accessed using standard LUA array syntax. Indexing starts at 1:

```
length = #sA \
sB[1] = sA[3]
```

The primitive number value of a scalar can be read and written with the ```value``` function,
and the individual rank in ```[0..255]``` with the ```rank``` function:

```
sA[1]:value()
sA[1]:value(4.0)
sA[1]:rank()
sA[1]:rank(128)
```

We differentiate between *reference* and *value* access.
The index operator ```[]``` always returns a reference so that the accessed Scalar can be modified,
while operations (that don't point to any particular scalar anymore) return a value. Avoid storing
references in variables to avoid ambiguities:

```
local X = sA[1]         -- X is now a reference to the first element of sA
X.value(23)             -- The first element of sA is now 23

local Y = sA[1] + sB[1] -- Y is just a value
Y.value(23)             -- sA/sB remains unchanged
```

Basic math operations are applied pairwise. If both scalars have different cardinality they are cropped to match. The ranks between two value pairs are minimized. The analysis results between two value pairs are maximized.

> \+ \- \* \/ \^ \% \< \> \<= \>= \==

LUA doesn't allow operations between variables of different type. Instead we support direct multiplication or addition with a number using the ```scale``` function for multiplication and ```shift``` function for addition:

> sA:scale ( 2.0 ) \
> sA:shift ( 4.0 )

## Lines

Lines represent array value pins, but since we support multiple values per pin they act like a twodimensional list of numbers.
A lot of the behaviour is equivalent to [scalars](#scalars) and not mentioned here again, notably ```[]``` operator access work the same.

Input and output pins are:

> lnA lnB \
> lnR lnS

Access to individual line values is also done via the ```[]``` operator, which returns [scalar](#scalars) references.

> lnA[1][1]:value ( ) \
> lnA[1][1]:rank ( 128 ) \
> lnB[1][1] = lnA[1][2] + lnB[1][2]

Similar to [scalar](#scalars) there are ```scale``` and ```shift``` functions, however lines also support [scalar](#scalars) values on top of just number primitives. If the scalar has only one value it will applied to all values, otherwise the values will be applied pairwise and cropped if they have different cardinality.

> lnA:scale ( sA ) \
> lnA:shift ( sB )

## Images

Images represent a rasterized pixel frame. Only single images are supported (unlike [scalars](#scalars) and [lines](#lines)), as a result the ```[]``` operator is not available.

Input and output pins are:

> imgA imgB \
> imgR imgS

Width and height of the image:

> imgA:width() \
> imgA:height() \

Access to the pixel values at the given location is given with the ```get``` and ```set``` functions.
The values are integers in ```[0 .. 255]```:

> imgA:get ( x, y ) \
> imgA:set ( x, y, v ) \

Some but not all math operations are supported. Notably, pixels cannot be negative and comparisons don´t make a lot of sense:

> \+ \- \* \/ \^ \%

Similar to [lines](#lines) images also support Scalar input, but only the first value of the scalar will be applied.

> imgA:scale ( sA ) \
> imgA:shift ( sB )

## Globals

### _N

Global integer variable denoting the current frame number (or iteration count) starting at 1.

### _C

Global variable holding the default image context. The default context is determined by the last context of the input pins, i.e. from the image pins if connected, otherwise lines, otherwise scalars.

If no Pin is connected the LUA node is not executed.

All variables (Scalar, Line, Image) have a ```context``` function returning the context they got created with.

### _Draw

Global variable supporting several draw operations:

> Draw:point ( layer, x, y, color ) \
> Draw:line ( layer, x0, y0, x1, y1, color ) \
> Draw:cross ( layer, x, y, radius, color ) \
> Draw:rect ( layer, x, y, w, h, color ) \
> Draw:text ( layer, x, y, text, size, color ) \
> Draw:circle ( layer, x, y, radius, color )

All drawing operations are applied after the filter has finished processing.

Layer arguments determine the draw order of shapes and can be written as integers ```[0 .. 10]``` or as one of the [_Layer constants](#Constants).

Color arguments can be written in hex (e.g. ```0xFF00FFFF```) or as one of the [_Color constants](#Constants).

### Constants

Constants are available globally and can be accessed with the dot operator like so:
> _Colors.red \
> _Results.analysisOk \
> _Ranks.min \
> _Math.pi

| Global        | Constant          | Value         |
| ------------- | ----------------- | ------------- |
| _Results      | analysisOk        | internal      |
|               | luaError          | internal      |
| _Scores       | NotPresent	    | 0.0           |
|               | Minimum		    | 0.01          |
|               | Marginal          | 0.1           |
|               | Bad			    | 0.25          |
|               | Doubtful	        | 0.4           |
|               | NotGood		    | 0.5			|
|               | Ok			    | 0.60          |
|               | Good		        | 0.75          |
|               | Perfect		    | 0.9           |
|               | Limit		        | 1.0           |
| _Ranks        | min               | 0             |
|               | max               | 255           |
| _Math         | pi 		        | 3.14159...    |
|               | sqrt2 	        | 1.41421...    |
|               | e 		        | 2.71828...    |
|               | ln2 	            | 0.69314...    |
| _Colors       | red               | FF0000        |
|               | green             | 00FF00        |
|               | blue              | 0000FF        |
|               | white             | FFFFFF        |
|               | black             | 000000        |
|               | yellow            | FFFF00        |
|               | cyan              | 00FFFF        |
|               | orange            | FFA500        |
|               | magenta           | FF00FF        |
| _Layers       | line              | 0             |
|               | contour           | 1             |
|               | position          | 2             |
|               | text              | 3             |
|               | gridtransp        | 4             |
|               | linetransp        | 5             |
|               | contourtransp     | 6             |
|               | positiontransp    | 7             |
|               | texttransp        | 8             |
|               | infobox           | 9             |
|               | image             | 10            |
