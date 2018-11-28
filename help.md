# Supported markdown

#help

## Headers
## h2
### h3
#### h4
##### h5
###### h6

## Lists

* list
    * item
* list
    * item
        * item
            1. item
                * item
* list

1. list
    2. item

## Text formatting

**Lorem ipsum dolor sit amet consectetur adipisicing elit.** Impedit, earum. Magnam, *distinctio nobis cum* nostrum unde perferendis ad earum facere reiciendis autem `voluptatem` eveniet iure corrupti, adipisci *expedita excepturi vel, **temporibus** nesciunt quod. Quod omnis dolore illo libero `quis` ad cupiditate nemo earum.* Voluptates sint neque molestias qui iste sed aperiam adipisci, quam possimus, ab magnam impedit mollitia autem non officiis dolorum libero cumque maxime rem vitae sequi. [Link](http://somesite.com) <!--Lorem--> **ipsum** dolor sit amet consectetur adipisicing elit. Impedit, earum. 

<!-- comment -->

## Code blocks

```^
    this is a code block
```

```c++^
int main() {
    std::cout << "this is a code block..." << std::endl;
    return 0;
}
```

```^
    System.out.println("hello, world!");
```

## Images and links

![image](https://dhamith93.github.io/I'm_A_Huge_Metal_Fan.jpg)

[Link](http://somesite.com)


## Tables

| Example        | Table                  |  one          |
| -------------- |:----------------------:| -------------:|
| default left   | centered               | right aligned |
| test text      |                        |     test text |
| test text      | this text is centered! | test text     |

This also works...

|Example|Table|one|
|---|:---:|---:|
|default left|centered|right aligned|
|test text| |test text|
|test text|this text is centered!|test text|

And this works too. But without alignment options.

|Example|Table|one|
|default left|centered|right aligned|
|test text| |test text|
|test text|this text is centered!|test text|