@echo off

call "../../compiler" input:terrain.vs type:vertex output:shader.vs
call "../../compiler" input:terrain.fs type:fragment output:shader.fs