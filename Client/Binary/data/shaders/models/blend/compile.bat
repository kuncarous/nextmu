@echo off

call "../../compiler" input:shader.vs type:vertex output:shader.vs
call "../../compiler" input:shader.fs type:fragment output:shader.fs