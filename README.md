# boxpush

![Downloads](https://img.shields.io/github/downloads/jibstack64/boxpush/total)
![GitHub tag (latest by date)](https://img.shields.io/badge/tag-0.3b-success)

#### A fun command-line game where you push boxes!
> If you encounter any bugs whilst playing `boxpush`, please [make an issue](https://github.com/jibstack64/boxpush/issues)!

![image](https://user-images.githubusercontent.com/107510599/200847018-1c50e938-d1d7-41f4-a105-93d45e066f58.png)

### How to play?
- You, the smiley face, push boxes across the map onto the crosses.
- When a point is scored, the box turns green and the `Score` counter is incremented by one.
- A big mistake is pushing a box onto the wall - doing this will prevent you from moving it off.
- Once all boxes are on a target, you will be taken to the next level.

### Controls
- `w/a/s/d` - Up, down, left, right.
- `r` - Resets the level you are on.

### Parameters
The executable has a series of parameters that can be used to customise the game. These include:
- `--map <width,height>` - removes the default maps and appends a newly generated one with the height and width provided.
> **Coming soon:**  
> `--from-file <path>` - loads a map from the file provided. 
- *The override parameters replace the default characters with the ones provided.*
  - `--override-player <text>`
  - `--override-box <text>`
  - `--override-target <text>`
  - `--override-border <text>`
  - `--override-background <text>`
  - `--override-tick <text>`

### Extra
There are `Windows` and `*nix` binaries in each release. The Windows binary is substantially larger because
I compile it with the `-static` flag (I am a Linux dev, compile it yourself, nerd).
The Windows build uses non-unicode characters as the (rather mediocre) Command Prompt does not support them.
