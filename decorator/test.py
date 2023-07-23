from decorator import decorator
from time import sleep, time

@decorator
def timer(func, repeat=1):
  def wrapper(*args, **kwargs):
    start = time()
    for _ in range(repeat):
      func(*args, **kwargs)
    print(f'Elapsed time: {time() - start}')
  return wrapper

@timer()
def sleep_1s():
  sleep(0.1)
  
sleep_1s()
print(sleep_1s.__name__)