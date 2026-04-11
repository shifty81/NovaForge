using System;
using Xunit;
using NovaForge.Core.Events;

namespace NovaForge.Tests
{
    public class EventBusTests
    {
        private struct TestEvent
        {
            public int Value;
        }

        private struct OtherEvent
        {
            public string Tag;
        }

        [Fact]
        public void Publish_DeliversEventToSubscriber()
        {
            var bus = new EventBus();
            int received = 0;
            bus.Subscribe<TestEvent>(e => received = e.Value);

            bus.Publish(new TestEvent { Value = 42 });

            Assert.Equal(42, received);
        }

        [Fact]
        public void Publish_DeliversToAllSubscribers()
        {
            var bus = new EventBus();
            int sum = 0;
            bus.Subscribe<TestEvent>(e => sum += e.Value);
            bus.Subscribe<TestEvent>(e => sum += e.Value);

            bus.Publish(new TestEvent { Value = 5 });

            Assert.Equal(10, sum);
        }

        [Fact]
        public void Unsubscribe_PreventsDelivery()
        {
            var bus = new EventBus();
            int received = 0;
            Action<TestEvent> handler = e => received = e.Value;
            bus.Subscribe(handler);
            bus.Unsubscribe(handler);

            bus.Publish(new TestEvent { Value = 99 });

            Assert.Equal(0, received);
        }

        [Fact]
        public void Publish_DoesNotDeliverToWrongEventType()
        {
            var bus = new EventBus();
            int received = 0;
            bus.Subscribe<TestEvent>(e => received = e.Value);

            bus.Publish(new OtherEvent { Tag = "hello" });

            Assert.Equal(0, received);
        }

        [Fact]
        public void Publish_NoSubscribers_DoesNotThrow()
        {
            var bus = new EventBus();
            // Should complete without exception.
            bus.Publish(new TestEvent { Value = 1 });
        }
    }
}
