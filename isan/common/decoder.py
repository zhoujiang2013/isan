import isan.common.pushdown as pushdown
import isan.common.dfabeam as dfabeam


class Searcher:
    raw_to_steps=lambda self,x:len(x)+1

    def set_action(self,action,d):
        self.searcher.set_action(self.handler,action,d)
    def export_weights(self):
        return self.searcher.export_weights(self.handler)
    def average_weights(self,step):
        self.searcher.average_weights(self.handler,step)
    def sum_weights(self,stat,action):
        return self.searcher.sum_weights(self.handler,stat,action)
    def un_average_weights(self):
        self.searcher.un_average_weights(self.handler)

    def update_action(self,move,delta,step):
        self.searcher.update_action(self.handler,move[0],move[1],delta,step)

    def make_dat(self):
        self.searcher.make_dat(self.handler)
        
    def get_states(self):
        return self.searcher.get_states(self.handler)

    def set_raw(self,raw):
        self.raw=raw
        self.searcher.set_raw(self.handler,raw)
    def __call__(self):
        return self.searcher.search(self.handler,self.raw_to_steps(self.raw))
    def search(self):
        #self.searcher.search(self.handler,self.raw_to_steps(self.raw))
        return self.searcher.search(self.handler,self.raw_to_steps(self.raw))

    def __del__(self):
        self.searcher.delete(self.handler)

class DFA(Searcher):
    name='状态转移'
    searcher=dfabeam
    raw_to_steps=lambda self,x:len(x)+1
    def __init__(self,schema,beam_width):
        self.handler=self.searcher.new(
                beam_width,
                schema.init_stat,
                schema.early_stop if hasattr(schema,'early_stop') else 0,
                schema.gen_actions_and_stats,
                schema.gen_features,
                )
    def search(self):
        return self.searcher.search(self.handler,self.raw_to_steps(self.raw))
        actions,states=self.searcher.search(self.handler,self.raw_to_steps(self.raw))
        print(actions)
        print(states)
        return actions
class Push_Down(Searcher):
    name='Shift-reduce'
    searcher=pushdown
    raw_to_steps=lambda self,x:2*len(x)-1
    def __init__(self,schema,beam_width):
        self.handler=self.searcher.new(
                beam_width,
                schema.init_stat,
                schema.early_stop,
                schema.shift,
                schema.reduce,
                schema.gen_features,
                )
