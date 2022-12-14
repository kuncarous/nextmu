@echo off

call "../../compiler" input:model.vs type:vertex output:shader.vs
call "../../compiler" input:model.fs type:fragment output:shader.fs