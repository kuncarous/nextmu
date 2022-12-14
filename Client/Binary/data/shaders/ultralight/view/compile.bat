@echo off

call "../../compiler" input:view.vs type:vertex output:shader.vs
call "../../compiler" input:view.fs type:fragment output:shader.fs