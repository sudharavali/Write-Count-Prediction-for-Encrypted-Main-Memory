import os, sys, re, os.path
import subprocess, datetime, time, signal
import matplotlib.pyplot as plt

parameter_file = ["parameter_std.hpp", "parameter.hpp"]
predictRate = []
# Cache Size
# History Size
# Set Size
parameter = [1,2,3,4]
def replace(filename, pattern, replacement):
    f = open(filename)
    s = f.read()
    f.close()
    s = re.sub(pattern, replacement, s)
    f = open(filename,'w')
    f.write(s)
    f.close()

def insert_job(jobs, param):
    insert_job.counter += 1
    jobs[insert_job.counter] = {
            "PARAM_CACHE_SIZE" : "32768",
            "PARAM_WCHISTORY_SIZE" : param,
            "PARAM_SET_SIZE" : "16",
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

    print "PASS Compile\t\thistory size=%s" %(job['PARAM_WCHISTORY_SIZE'])

def plotgraph(parameter):
    fig = plt.figure()
    title = "Prediction_vs_HistorySize"
    ax = fig.add_subplot(111)
    ax.plot(parameter, predictRate, marker='o')
    ax.set_ylabel('Total Cache Lines')
    ax.set_xlabel('WC History Size')
    fig.savefig(title+'.png')

def run_test():
    cmd = "./cacheModel"
    start = datetime.datetime.now()
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    while process.poll() is None:
        time.sleep(1)
        now = datetime.datetime.now()
    
    stdout = process.stdout.readlines()
    PASS = False
    for line in stdout:
        if "PASS" in line:
            print "Execution PASS"
            PASS = True
        if PASS and "Summary:" in line:
            correctPrediction = float(re.search("(?<=CorrectPrediction=)[0-9]*", line).group(0))
            totalPrediction = float(re.search("(?<=TotalPrediction=)[0-9]*", line).group(0))
            rate = correctPrediction/totalPrediction * 100
            predictRate.append(rate)

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

    for param in parameter:
        insert_job(jobs, param)

    run_all(jobs)
    plotgraph(parameter)
    os.system('make clean > temp.out 2>&1')
    os.system('rm -rf temp.out')

if '__main__' == __name__:
    
    run_benchmark()


    


