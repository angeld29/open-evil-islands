import os
import sys
import subprocess

if len(sys.argv) != 2:
	print "Usage: %s <mmp dir>" % sys.argv[0]
	sys.exit()

for dir_name in os.listdir(sys.argv[1]):
	dir_path = os.path.join(sys.argv[1], dir_name)
	for file_name in os.listdir(dir_path):
		file_path = os.path.join(dir_path, file_name)
		prog_path = os.path.join("test", "bin",
			os.path.splitext(file_name)[1][1:])
		prog = subprocess.Popen([prog_path, file_path], stdout=subprocess.PIPE)
		out = prog.communicate()[0]
		with open(file_path.replace('.', '_'), "wt") as f:
			f.write(out)
