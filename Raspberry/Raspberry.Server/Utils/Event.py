class Event(object): 
	def __init__(self): 
		self.__eventhandlers = [] 
	
	def __iadd__(self, Ehandler): 
		self.__eventhandlers.append(Ehandler) 
		return self
	
	def __isub__(self, Ehandler): 
		self.__eventhandlers.remove(Ehandler) 
		return self

	def __call__(self, *args, **keywargs): 
		for handler in self.__eventhandlers: 
			handler(*args, **keywargs) 
