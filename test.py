import os, sys, re, os.path
import subprocess, datetime, time, signal

parameter_file = ["parameter_std.hpp", "parameter.hpp"]

csize = [2, 4, 8]
def replace(filename, pattern, replacement):
    f = open(filename)
    s = f.read()
    f.close()
    s = re.sub(pattern, replacement, s)
    f = open(filename,'w')
    f.write(s)
    f.close()

def insert_job(jobs, cache_size):
    insert_job.counter += 1
    jobs[insert_job.counter] = {
            "PARAM_CACHE_SIZE" : cache_size,
            }

def compile(job):
    os.system("cp " + parameter_file[0] + ' ' + parameter_file[1])
    for (param, value) in job.iteritems():
        pattern = r"\#define\s*" + re.escape(param) + r'.*'
        replacement = "#define " + param + ' ' + str(value)
        replace(parameter_file[1], pattern, replacement)
    os.system("make clean > temp.out 2>&1")
    ret = os.system("make all > temp.out 2>&1")
    if ret != 0:
        print "ERROR in compiling job= "
        print job
        exit(0)

    print "PASS Compile\t\tcache_size=%s" %(job['PARAM_CACHE_SIZE'])

def run_test():
    cmd = "./cacheModel"
    start = datetime.datetime.now()
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    timeout = 10
    while process.poll() is None:
        time.sleep(1)
        now = datetime.datetime.now()
        if (now - start).seconds > timeout:
            os.kill(process.pid, signal.SIGKILL)
            os.waitpid(-1, os.WHOHANG)
            print "ERROR: Timeout cmd=%s" %cmd
            exit(0)
    
    stdout = process.stdout.readlines()
    PASS = False
    for line in stdout:
        if "PASS" in line:
            print "Execution PASS"
            PASS = True
        if PASS and "Summary:" in line:
            correctPrediction = int(re.search("(?<=CorrectPrediction=)[0-9]*", line).group(0))
            totalPrediction = int(re.search("(?<=TotalPrediction=)[0-9]*", line).group(0))
            print correctPrediction
            print totalPrediction
            return
    print "FAILED execution"
    exit(0)



def run_all(jobs):
    for (jobname, job) in jobs.iteritems():
        compile(job)
        run_test()


def run_benchmark():
    insert_job.counter = 0
    jobs = {}

    for size in csize:
        insert_job(jobs, size)

    run_all(jobs)
    os.system('make clean > temp.out 2>&1')
    os.system('rm -rf temp.out')

if '__main__' == __name__:
    
    run_benchmark()


    


