import asyncio
from unittest import result
 
async def task1():
    print("do something with userinput1...")
    
async def task2():
    print("do something with userinput2...")

async def task3():
    print("do something with userinput3...")

async def main():
    await asyncio.wait([task1() , task2(), task3()])

async def function(user_input, input_index):
    print(f'In {input_index} function: {user_input}')    

async def main2():
    tasks = []
    for input_index in range(1, 4):
        user_input = input(f"Enter input #{input_index}\n")
        tasks.append(asyncio.create_task(function(user_input, input_index)))
    await asyncio.gather(*tasks)

async def function1(user_input, input_index):
    print(f'In {input_index} function1: {user_input}')

async def function2(user_input, input_index):
    print(f'In {input_index} function2: {user_input}')

async def function3(user_input, input_index):
    print(f'In {input_index} function3: {user_input}')

FUNCTION_DICTIONARY = { 1 : function1, 2 : function2, 3 : function3 }

async def main3():
    tasks = []
    for input_index in range(1, 4):
        user_input = input(f"Enter input #{input_index}\n")
        tasks.append(asyncio.create_task(FUNCTION_DICTIONARY[input_index](user_input, input_index)))
    await asyncio.gather(*tasks)

inputs = ['a', 'b', 'c']

async def task(input: str):
    # Do stuff / await stuff
    return input
   
async def main4():
    resuts = await asyncio.wait(
        [task(arg) for arg in inputs]
    )
    r = result

async def func_normal():
    print("A")
    await asyncio.sleep(5)
    print("B")
    return 'X'

async def func_infinite():
    return('Y')

def main5():
    # print(await func_normal())
    tasks = func_normal(), func_infinite()
    loop = asyncio.get_event_loop()
    tasks = func_normal(), func_infinite()
    result = loop.run_until_complete(asyncio.gather(*tasks))
    # reuslt = asyncio.wait(tasks)
    print("func_normal()={result[0]}, func_infinite()={result[1]}".format(**vars()))
    loop.close()

async def factorial(name, number):
    f = 1
    for i in range(2, number + 1):
        print(f"Task {name}: Compute factorial({number}), currently i={i}...")
        await asyncio.sleep(1)
        f *= i
    print(f"Task {name}: factorial({number}) = {f}")
    return f

async def main6():
    # Schedule three calls *concurrently*:
    tasks =  factorial("A", 2),  factorial("B", 3), factorial("C", 4)
    L = await asyncio.gather(*tasks)
    print(L)

if __name__ == '__main__':
    # asyncio.get_event_loop().run_until_complete(main())
    # asyncio.get_event_loop().run_until_complete(main2())
    asyncio.get_event_loop().run_until_complete(main3())
    # asyncio.run(main4())
    # asyncio.run(main5())
    # main5()
    # asyncio.run(main6())

    # loop = asyncio.new_event_loop()
    # asyncio.set_event_loop(loop)
    asyncio.get_event_loop().run_forever()
