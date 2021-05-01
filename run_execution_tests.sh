#!/usr/bin/env bash


oneTimeSetUp() {
    readonly EXEC_BIN="exec_test_bin"
    gcc -o "$EXEC_BIN" src/*.c -lreadline -I.

    # Commands are fed to a named pipe
    readonly TEST_SHELL="bunsh_fifo_pipe"
    mkfifo "$TEST_SHELL"

    # Processes created by the shell are isolated in a new session
    setsid ./"$EXEC_BIN" > /dev/null < "$TEST_SHELL" &

    readonly SHELL_PID=$!
    exec 3>"$TEST_SHELL"
}


testRedirection() {
    readonly REDIR_CMD="tee < $0"
    readonly REDIR_OUTPUT="redir_test_output"

    echo "$REDIR_CMD > $REDIR_OUTPUT" > "$TEST_SHELL"

    waitForFileOutput "$REDIR_OUTPUT"

    # Should behave as in bash
    diff "$REDIR_OUTPUT" <(eval "$REDIR_CMD")
    assertTrue $?

    rm "$REDIR_OUTPUT"
}


testPipeline() {
    readonly PIPELINED_STR="hej"
    pipeline_cmd="echo $PIPELINED_STR"
    for _ in {0..10}; do
        pipeline_cmd="$pipeline_cmd | tee"
    done
    readonly PIPELINE_OUTPUT="pipeline_test_output"
    pipeline_cmd="$pipeline_cmd > $PIPELINE_OUTPUT"

    echo "$pipeline_cmd" > "$TEST_SHELL"

    waitForFileOutput "$PIPELINE_OUTPUT"

    diff "$PIPELINE_OUTPUT" <(echo "$PIPELINED_STR")
    assertTrue $?

    rm "$PIPELINE_OUTPUT"
}


testBackgroundProcessNotKilledByCtrlC() {
    echo "sleep 10 &" > "$TEST_SHELL"
    sleep 1
    # The test shell's sid and pgid are the same as its pid
    readonly PS_BEFORE_SIGINT=$(ps -s $SHELL_PID)
    kill -INT -$SHELL_PID
    sleep 1
    readonly PS_AFTER_SIGINT=$(ps -s $SHELL_PID)

    assertEquals "$PS_BEFORE_SIGINT" "$PS_AFTER_SIGINT"
}


testBackgroundProcessIsWaitedOn() {
    readonly BG_CMD="true &"
    echo "$BG_CMD" > "$TEST_SHELL"
    sleep 1
    readonly ZOMBIES=$(ps -o cmd,stat -s $SHELL_PID \
        | grep -E "defunct|Z")

    assertNull "$ZOMBIES"
}


# Returns when the file given by the first parameter has been created and written to
waitForFileOutput() {
    file="$1"
    while [ ! -s "$file" ]; do
        inotifywait -qqt 1 -e close_write .
    done
}


oneTimeTearDown() {
    rm "$EXEC_BIN"
    exec 3>&-
    rm "$TEST_SHELL"
}


. shunit2