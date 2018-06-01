# Bash

In your `~/.bashrc`:

```bash
bf () {
  bjam "$@" 2>&1 \
  | stdbuf -o0 $REDEMPTION_PATH/tools/bjam/bjam_filter.awk \
  | $REDEMPTION_PATH/tools/bjam/unit_test_color.sh -e "s#$PWD#./#g;s#$HOME/#~/#g"
  return ${PIPESTATUS[0]}
}

pbf='REDEMPTION_LOG_PRINT=1 bf'

source $REDEMPTION_PATH/tools/bjam/bjam_completion.bash
```

# Zsh

In your `~/.zshrc`:

```bash
bf () {
  bjam "$@" 2>&1 \
  | stdbuf -o0 $REDEMPTION_PATH/tools/bjam/bjam_filter.awk \
  | $REDEMPTION_PATH/tools/bjam/unit_test_color.sh -e "s#$PWD#./#g;s#$HOME/#~/#g"
  return ${pipestatus[1]}
}

pbf='REDEMPTION_LOG_PRINT=1 bf'

source $REDEMPTION_PATH/tools/bjam/bjam_completion.zsh
```
