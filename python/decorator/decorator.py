
from functools import wraps, update_wrapper

# makes it so decorator gets args and kwargs put into decorator
# returns passed decorator with args and kwargs
def decorator(decorator) -> callable:
  # decorator1 replaces passed decorator allowing it to get args and kwargs
  def decorator1(*args, **kwargs):
    @wraps(decorator)
    def decorator2(func):
      obj = decorator(func, *args, **kwargs)
      update_wrapper(obj, func)
      return obj
    return decorator2
  return decorator1