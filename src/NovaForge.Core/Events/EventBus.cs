using System;
using System.Collections.Generic;

namespace NovaForge.Core.Events
{
    public class EventBus
    {
        private readonly Dictionary<Type, List<Delegate>> _handlers = new Dictionary<Type, List<Delegate>>();

        public void Subscribe<T>(Action<T> handler)
        {
            Type t = typeof(T);
            if (!_handlers.ContainsKey(t))
                _handlers[t] = new List<Delegate>();
            _handlers[t].Add(handler);
        }

        public void Unsubscribe<T>(Action<T> handler)
        {
            Type t = typeof(T);
            if (_handlers.TryGetValue(t, out var list))
                list.Remove(handler);
        }

        public void Publish<T>(T evt)
        {
            Type t = typeof(T);
            if (_handlers.TryGetValue(t, out var list))
            {
                foreach (var d in list)
                    ((Action<T>)d)(evt);
            }
        }
    }
}
