using Bonsai;
using Bonsai.Harp;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Reactive.Linq;
using System.Xml.Serialization;

namespace AllenNeuralDynamics.LicketySplitLickDetector
{
    /// <summary>
    /// Generates events and processes commands for the LicketySplitLickDetector device connected
    /// at the specified serial port.
    /// </summary>
    [Combinator(MethodName = nameof(Generate))]
    [WorkflowElementCategory(ElementCategory.Source)]
    [Description("Generates events and processes commands for the LicketySplitLickDetector device.")]
    public partial class Device : Bonsai.Harp.Device, INamedElement
    {
        /// <summary>
        /// Represents the unique identity class of the <see cref="LicketySplitLickDetector"/> device.
        /// This field is constant.
        /// </summary>
        public const int WhoAmI = 1400;

        /// <summary>
        /// Initializes a new instance of the <see cref="Device"/> class.
        /// </summary>
        public Device() : base(WhoAmI) { }

        string INamedElement.Name => nameof(LicketySplitLickDetector);

        /// <summary>
        /// Gets a read-only mapping from address to register type.
        /// </summary>
        public static new IReadOnlyDictionary<int, Type> RegisterMap { get; } = new Dictionary<int, Type>
            (Bonsai.Harp.Device.RegisterMap.ToDictionary(entry => entry.Key, entry => entry.Value))
        {
            { 32, typeof(LickState) },
            { 33, typeof(Channel0TriggerThreshold) },
            { 34, typeof(Channel0UntriggerThreshold) }
        };
    }

    /// <summary>
    /// Represents an operator that groups the sequence of <see cref="LicketySplitLickDetector"/>" messages by register type.
    /// </summary>
    [Description("Groups the sequence of LicketySplitLickDetector messages by register type.")]
    public partial class GroupByRegister : Combinator<HarpMessage, IGroupedObservable<Type, HarpMessage>>
    {
        /// <summary>
        /// Groups an observable sequence of <see cref="LicketySplitLickDetector"/> messages
        /// by register type.
        /// </summary>
        /// <param name="source">The sequence of Harp device messages.</param>
        /// <returns>
        /// A sequence of observable groups, each of which corresponds to a unique
        /// <see cref="LicketySplitLickDetector"/> register.
        /// </returns>
        public override IObservable<IGroupedObservable<Type, HarpMessage>> Process(IObservable<HarpMessage> source)
        {
            return source.GroupBy(message => Device.RegisterMap[message.Address]);
        }
    }

    /// <summary>
    /// Represents an operator that filters register-specific messages
    /// reported by the <see cref="LicketySplitLickDetector"/> device.
    /// </summary>
    /// <seealso cref="LickState"/>
    /// <seealso cref="Channel0TriggerThreshold"/>
    /// <seealso cref="Channel0UntriggerThreshold"/>
    [XmlInclude(typeof(LickState))]
    [XmlInclude(typeof(Channel0TriggerThreshold))]
    [XmlInclude(typeof(Channel0UntriggerThreshold))]
    [Description("Filters register-specific messages reported by the LicketySplitLickDetector device.")]
    public class FilterRegister : FilterRegisterBuilder, INamedElement
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="FilterRegister"/> class.
        /// </summary>
        public FilterRegister()
        {
            Register = new LickState();
        }

        string INamedElement.Name
        {
            get => $"{nameof(LicketySplitLickDetector)}.{GetElementDisplayName(Register)}";
        }
    }

    /// <summary>
    /// Represents an operator which filters and selects specific messages
    /// reported by the LicketySplitLickDetector device.
    /// </summary>
    /// <seealso cref="LickState"/>
    /// <seealso cref="Channel0TriggerThreshold"/>
    /// <seealso cref="Channel0UntriggerThreshold"/>
    [XmlInclude(typeof(LickState))]
    [XmlInclude(typeof(Channel0TriggerThreshold))]
    [XmlInclude(typeof(Channel0UntriggerThreshold))]
    [XmlInclude(typeof(TimestampedLickState))]
    [XmlInclude(typeof(TimestampedChannel0TriggerThreshold))]
    [XmlInclude(typeof(TimestampedChannel0UntriggerThreshold))]
    [Description("Filters and selects specific messages reported by the LicketySplitLickDetector device.")]
    public partial class Parse : ParseBuilder, INamedElement
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="Parse"/> class.
        /// </summary>
        public Parse()
        {
            Register = new LickState();
        }

        string INamedElement.Name => $"{nameof(LicketySplitLickDetector)}.{GetElementDisplayName(Register)}";
    }

    /// <summary>
    /// Represents an operator which formats a sequence of values as specific
    /// LicketySplitLickDetector register messages.
    /// </summary>
    /// <seealso cref="LickState"/>
    /// <seealso cref="Channel0TriggerThreshold"/>
    /// <seealso cref="Channel0UntriggerThreshold"/>
    [XmlInclude(typeof(LickState))]
    [XmlInclude(typeof(Channel0TriggerThreshold))]
    [XmlInclude(typeof(Channel0UntriggerThreshold))]
    [Description("Formats a sequence of values as specific LicketySplitLickDetector register messages.")]
    public partial class Format : FormatBuilder, INamedElement
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="Format"/> class.
        /// </summary>
        public Format()
        {
            Register = new LickState();
        }

        string INamedElement.Name => $"{nameof(LicketySplitLickDetector)}.{GetElementDisplayName(Register)}";
    }

    /// <summary>
    /// Represents a register that emits an event when the state of any lick detector changes. Value will be High when lick detected and Low otherwise.
    /// </summary>
    [Description("Emits an event when the state of any lick detector changes. Value will be High when lick detected and Low otherwise.")]
    public partial class LickState
    {
        /// <summary>
        /// Represents the address of the <see cref="LickState"/> register. This field is constant.
        /// </summary>
        public const int Address = 32;

        /// <summary>
        /// Represents the payload type of the <see cref="LickState"/> register. This field is constant.
        /// </summary>
        public const PayloadType RegisterType = PayloadType.U8;

        /// <summary>
        /// Represents the length of the <see cref="LickState"/> register. This field is constant.
        /// </summary>
        public const int RegisterLength = 1;

        /// <summary>
        /// Returns the payload data for <see cref="LickState"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the message payload.</returns>
        public static LickChannels GetPayload(HarpMessage message)
        {
            return (LickChannels)message.GetPayloadByte();
        }

        /// <summary>
        /// Returns the timestamped payload data for <see cref="LickState"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the timestamped message payload.</returns>
        public static Timestamped<LickChannels> GetTimestampedPayload(HarpMessage message)
        {
            var payload = message.GetTimestampedPayloadByte();
            return Timestamped.Create((LickChannels)payload.Value, payload.Seconds);
        }

        /// <summary>
        /// Returns a Harp message for the <see cref="LickState"/> register.
        /// </summary>
        /// <param name="messageType">The type of the Harp message.</param>
        /// <param name="value">The value to be stored in the message payload.</param>
        /// <returns>
        /// A <see cref="HarpMessage"/> object for the <see cref="LickState"/> register
        /// with the specified message type and payload.
        /// </returns>
        public static HarpMessage FromPayload(MessageType messageType, LickChannels value)
        {
            return HarpMessage.FromByte(Address, messageType, (byte)value);
        }

        /// <summary>
        /// Returns a timestamped Harp message for the <see cref="LickState"/>
        /// register.
        /// </summary>
        /// <param name="timestamp">The timestamp of the message payload, in seconds.</param>
        /// <param name="messageType">The type of the Harp message.</param>
        /// <param name="value">The value to be stored in the message payload.</param>
        /// <returns>
        /// A <see cref="HarpMessage"/> object for the <see cref="LickState"/> register
        /// with the specified message type, timestamp, and payload.
        /// </returns>
        public static HarpMessage FromPayload(double timestamp, MessageType messageType, LickChannels value)
        {
            return HarpMessage.FromByte(Address, timestamp, messageType, (byte)value);
        }
    }

    /// <summary>
    /// Provides methods for manipulating timestamped messages from the
    /// LickState register.
    /// </summary>
    /// <seealso cref="LickState"/>
    [Description("Filters and selects timestamped messages from the LickState register.")]
    public partial class TimestampedLickState
    {
        /// <summary>
        /// Represents the address of the <see cref="LickState"/> register. This field is constant.
        /// </summary>
        public const int Address = LickState.Address;

        /// <summary>
        /// Returns timestamped payload data for <see cref="LickState"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the timestamped message payload.</returns>
        public static Timestamped<LickChannels> GetPayload(HarpMessage message)
        {
            return LickState.GetTimestampedPayload(message);
        }
    }

    /// <summary>
    /// Represents a register that threshold value to detect the lick. Values below this threshold will be considered a detected lick.
    /// </summary>
    [Description("Threshold value to detect the lick. Values below this threshold will be considered a detected lick.")]
    public partial class Channel0TriggerThreshold
    {
        /// <summary>
        /// Represents the address of the <see cref="Channel0TriggerThreshold"/> register. This field is constant.
        /// </summary>
        public const int Address = 33;

        /// <summary>
        /// Represents the payload type of the <see cref="Channel0TriggerThreshold"/> register. This field is constant.
        /// </summary>
        public const PayloadType RegisterType = PayloadType.U8;

        /// <summary>
        /// Represents the length of the <see cref="Channel0TriggerThreshold"/> register. This field is constant.
        /// </summary>
        public const int RegisterLength = 1;

        /// <summary>
        /// Returns the payload data for <see cref="Channel0TriggerThreshold"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the message payload.</returns>
        public static byte GetPayload(HarpMessage message)
        {
            return message.GetPayloadByte();
        }

        /// <summary>
        /// Returns the timestamped payload data for <see cref="Channel0TriggerThreshold"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the timestamped message payload.</returns>
        public static Timestamped<byte> GetTimestampedPayload(HarpMessage message)
        {
            return message.GetTimestampedPayloadByte();
        }

        /// <summary>
        /// Returns a Harp message for the <see cref="Channel0TriggerThreshold"/> register.
        /// </summary>
        /// <param name="messageType">The type of the Harp message.</param>
        /// <param name="value">The value to be stored in the message payload.</param>
        /// <returns>
        /// A <see cref="HarpMessage"/> object for the <see cref="Channel0TriggerThreshold"/> register
        /// with the specified message type and payload.
        /// </returns>
        public static HarpMessage FromPayload(MessageType messageType, byte value)
        {
            return HarpMessage.FromByte(Address, messageType, value);
        }

        /// <summary>
        /// Returns a timestamped Harp message for the <see cref="Channel0TriggerThreshold"/>
        /// register.
        /// </summary>
        /// <param name="timestamp">The timestamp of the message payload, in seconds.</param>
        /// <param name="messageType">The type of the Harp message.</param>
        /// <param name="value">The value to be stored in the message payload.</param>
        /// <returns>
        /// A <see cref="HarpMessage"/> object for the <see cref="Channel0TriggerThreshold"/> register
        /// with the specified message type, timestamp, and payload.
        /// </returns>
        public static HarpMessage FromPayload(double timestamp, MessageType messageType, byte value)
        {
            return HarpMessage.FromByte(Address, timestamp, messageType, value);
        }
    }

    /// <summary>
    /// Provides methods for manipulating timestamped messages from the
    /// Channel0TriggerThreshold register.
    /// </summary>
    /// <seealso cref="Channel0TriggerThreshold"/>
    [Description("Filters and selects timestamped messages from the Channel0TriggerThreshold register.")]
    public partial class TimestampedChannel0TriggerThreshold
    {
        /// <summary>
        /// Represents the address of the <see cref="Channel0TriggerThreshold"/> register. This field is constant.
        /// </summary>
        public const int Address = Channel0TriggerThreshold.Address;

        /// <summary>
        /// Returns timestamped payload data for <see cref="Channel0TriggerThreshold"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the timestamped message payload.</returns>
        public static Timestamped<byte> GetPayload(HarpMessage message)
        {
            return Channel0TriggerThreshold.GetTimestampedPayload(message);
        }
    }

    /// <summary>
    /// Represents a register that threshold value to release the lick detection state. Values above this threshold will untrigger a detected lick.
    /// </summary>
    [Description("Threshold value to release the lick detection state. Values above this threshold will untrigger a detected lick.")]
    public partial class Channel0UntriggerThreshold
    {
        /// <summary>
        /// Represents the address of the <see cref="Channel0UntriggerThreshold"/> register. This field is constant.
        /// </summary>
        public const int Address = 34;

        /// <summary>
        /// Represents the payload type of the <see cref="Channel0UntriggerThreshold"/> register. This field is constant.
        /// </summary>
        public const PayloadType RegisterType = PayloadType.U8;

        /// <summary>
        /// Represents the length of the <see cref="Channel0UntriggerThreshold"/> register. This field is constant.
        /// </summary>
        public const int RegisterLength = 1;

        /// <summary>
        /// Returns the payload data for <see cref="Channel0UntriggerThreshold"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the message payload.</returns>
        public static byte GetPayload(HarpMessage message)
        {
            return message.GetPayloadByte();
        }

        /// <summary>
        /// Returns the timestamped payload data for <see cref="Channel0UntriggerThreshold"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the timestamped message payload.</returns>
        public static Timestamped<byte> GetTimestampedPayload(HarpMessage message)
        {
            return message.GetTimestampedPayloadByte();
        }

        /// <summary>
        /// Returns a Harp message for the <see cref="Channel0UntriggerThreshold"/> register.
        /// </summary>
        /// <param name="messageType">The type of the Harp message.</param>
        /// <param name="value">The value to be stored in the message payload.</param>
        /// <returns>
        /// A <see cref="HarpMessage"/> object for the <see cref="Channel0UntriggerThreshold"/> register
        /// with the specified message type and payload.
        /// </returns>
        public static HarpMessage FromPayload(MessageType messageType, byte value)
        {
            return HarpMessage.FromByte(Address, messageType, value);
        }

        /// <summary>
        /// Returns a timestamped Harp message for the <see cref="Channel0UntriggerThreshold"/>
        /// register.
        /// </summary>
        /// <param name="timestamp">The timestamp of the message payload, in seconds.</param>
        /// <param name="messageType">The type of the Harp message.</param>
        /// <param name="value">The value to be stored in the message payload.</param>
        /// <returns>
        /// A <see cref="HarpMessage"/> object for the <see cref="Channel0UntriggerThreshold"/> register
        /// with the specified message type, timestamp, and payload.
        /// </returns>
        public static HarpMessage FromPayload(double timestamp, MessageType messageType, byte value)
        {
            return HarpMessage.FromByte(Address, timestamp, messageType, value);
        }
    }

    /// <summary>
    /// Provides methods for manipulating timestamped messages from the
    /// Channel0UntriggerThreshold register.
    /// </summary>
    /// <seealso cref="Channel0UntriggerThreshold"/>
    [Description("Filters and selects timestamped messages from the Channel0UntriggerThreshold register.")]
    public partial class TimestampedChannel0UntriggerThreshold
    {
        /// <summary>
        /// Represents the address of the <see cref="Channel0UntriggerThreshold"/> register. This field is constant.
        /// </summary>
        public const int Address = Channel0UntriggerThreshold.Address;

        /// <summary>
        /// Returns timestamped payload data for <see cref="Channel0UntriggerThreshold"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the timestamped message payload.</returns>
        public static Timestamped<byte> GetPayload(HarpMessage message)
        {
            return Channel0UntriggerThreshold.GetTimestampedPayload(message);
        }
    }

    /// <summary>
    /// Represents an operator which creates standard message payloads for the
    /// LicketySplitLickDetector device.
    /// </summary>
    /// <seealso cref="CreateLickStatePayload"/>
    /// <seealso cref="CreateChannel0TriggerThresholdPayload"/>
    /// <seealso cref="CreateChannel0UntriggerThresholdPayload"/>
    [XmlInclude(typeof(CreateLickStatePayload))]
    [XmlInclude(typeof(CreateChannel0TriggerThresholdPayload))]
    [XmlInclude(typeof(CreateChannel0UntriggerThresholdPayload))]
    [XmlInclude(typeof(CreateTimestampedLickStatePayload))]
    [XmlInclude(typeof(CreateTimestampedChannel0TriggerThresholdPayload))]
    [XmlInclude(typeof(CreateTimestampedChannel0UntriggerThresholdPayload))]
    [Description("Creates standard message payloads for the LicketySplitLickDetector device.")]
    public partial class CreateMessage : CreateMessageBuilder, INamedElement
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="CreateMessage"/> class.
        /// </summary>
        public CreateMessage()
        {
            Payload = new CreateLickStatePayload();
        }

        string INamedElement.Name => $"{nameof(LicketySplitLickDetector)}.{GetElementDisplayName(Payload)}";
    }

    /// <summary>
    /// Represents an operator that creates a message payload
    /// that emits an event when the state of any lick detector changes. Value will be High when lick detected and Low otherwise.
    /// </summary>
    [DisplayName("LickStatePayload")]
    [Description("Creates a message payload that emits an event when the state of any lick detector changes. Value will be High when lick detected and Low otherwise.")]
    public partial class CreateLickStatePayload
    {
        /// <summary>
        /// Gets or sets the value that emits an event when the state of any lick detector changes. Value will be High when lick detected and Low otherwise.
        /// </summary>
        [Description("The value that emits an event when the state of any lick detector changes. Value will be High when lick detected and Low otherwise.")]
        public LickChannels LickState { get; set; }

        /// <summary>
        /// Creates a message payload for the LickState register.
        /// </summary>
        /// <returns>The created message payload value.</returns>
        public LickChannels GetPayload()
        {
            return LickState;
        }

        /// <summary>
        /// Creates a message that emits an event when the state of any lick detector changes. Value will be High when lick detected and Low otherwise.
        /// </summary>
        /// <param name="messageType">Specifies the type of the created message.</param>
        /// <returns>A new message for the LickState register.</returns>
        public HarpMessage GetMessage(MessageType messageType)
        {
            return AllenNeuralDynamics.LicketySplitLickDetector.LickState.FromPayload(messageType, GetPayload());
        }
    }

    /// <summary>
    /// Represents an operator that creates a timestamped message payload
    /// that emits an event when the state of any lick detector changes. Value will be High when lick detected and Low otherwise.
    /// </summary>
    [DisplayName("TimestampedLickStatePayload")]
    [Description("Creates a timestamped message payload that emits an event when the state of any lick detector changes. Value will be High when lick detected and Low otherwise.")]
    public partial class CreateTimestampedLickStatePayload : CreateLickStatePayload
    {
        /// <summary>
        /// Creates a timestamped message that emits an event when the state of any lick detector changes. Value will be High when lick detected and Low otherwise.
        /// </summary>
        /// <param name="timestamp">The timestamp of the message payload, in seconds.</param>
        /// <param name="messageType">Specifies the type of the created message.</param>
        /// <returns>A new timestamped message for the LickState register.</returns>
        public HarpMessage GetMessage(double timestamp, MessageType messageType)
        {
            return AllenNeuralDynamics.LicketySplitLickDetector.LickState.FromPayload(timestamp, messageType, GetPayload());
        }
    }

    /// <summary>
    /// Represents an operator that creates a message payload
    /// that threshold value to detect the lick. Values below this threshold will be considered a detected lick.
    /// </summary>
    [DisplayName("Channel0TriggerThresholdPayload")]
    [Description("Creates a message payload that threshold value to detect the lick. Values below this threshold will be considered a detected lick.")]
    public partial class CreateChannel0TriggerThresholdPayload
    {
        /// <summary>
        /// Gets or sets the value that threshold value to detect the lick. Values below this threshold will be considered a detected lick.
        /// </summary>
        [Description("The value that threshold value to detect the lick. Values below this threshold will be considered a detected lick.")]
        public byte Channel0TriggerThreshold { get; set; }

        /// <summary>
        /// Creates a message payload for the Channel0TriggerThreshold register.
        /// </summary>
        /// <returns>The created message payload value.</returns>
        public byte GetPayload()
        {
            return Channel0TriggerThreshold;
        }

        /// <summary>
        /// Creates a message that threshold value to detect the lick. Values below this threshold will be considered a detected lick.
        /// </summary>
        /// <param name="messageType">Specifies the type of the created message.</param>
        /// <returns>A new message for the Channel0TriggerThreshold register.</returns>
        public HarpMessage GetMessage(MessageType messageType)
        {
            return AllenNeuralDynamics.LicketySplitLickDetector.Channel0TriggerThreshold.FromPayload(messageType, GetPayload());
        }
    }

    /// <summary>
    /// Represents an operator that creates a timestamped message payload
    /// that threshold value to detect the lick. Values below this threshold will be considered a detected lick.
    /// </summary>
    [DisplayName("TimestampedChannel0TriggerThresholdPayload")]
    [Description("Creates a timestamped message payload that threshold value to detect the lick. Values below this threshold will be considered a detected lick.")]
    public partial class CreateTimestampedChannel0TriggerThresholdPayload : CreateChannel0TriggerThresholdPayload
    {
        /// <summary>
        /// Creates a timestamped message that threshold value to detect the lick. Values below this threshold will be considered a detected lick.
        /// </summary>
        /// <param name="timestamp">The timestamp of the message payload, in seconds.</param>
        /// <param name="messageType">Specifies the type of the created message.</param>
        /// <returns>A new timestamped message for the Channel0TriggerThreshold register.</returns>
        public HarpMessage GetMessage(double timestamp, MessageType messageType)
        {
            return AllenNeuralDynamics.LicketySplitLickDetector.Channel0TriggerThreshold.FromPayload(timestamp, messageType, GetPayload());
        }
    }

    /// <summary>
    /// Represents an operator that creates a message payload
    /// that threshold value to release the lick detection state. Values above this threshold will untrigger a detected lick.
    /// </summary>
    [DisplayName("Channel0UntriggerThresholdPayload")]
    [Description("Creates a message payload that threshold value to release the lick detection state. Values above this threshold will untrigger a detected lick.")]
    public partial class CreateChannel0UntriggerThresholdPayload
    {
        /// <summary>
        /// Gets or sets the value that threshold value to release the lick detection state. Values above this threshold will untrigger a detected lick.
        /// </summary>
        [Description("The value that threshold value to release the lick detection state. Values above this threshold will untrigger a detected lick.")]
        public byte Channel0UntriggerThreshold { get; set; }

        /// <summary>
        /// Creates a message payload for the Channel0UntriggerThreshold register.
        /// </summary>
        /// <returns>The created message payload value.</returns>
        public byte GetPayload()
        {
            return Channel0UntriggerThreshold;
        }

        /// <summary>
        /// Creates a message that threshold value to release the lick detection state. Values above this threshold will untrigger a detected lick.
        /// </summary>
        /// <param name="messageType">Specifies the type of the created message.</param>
        /// <returns>A new message for the Channel0UntriggerThreshold register.</returns>
        public HarpMessage GetMessage(MessageType messageType)
        {
            return AllenNeuralDynamics.LicketySplitLickDetector.Channel0UntriggerThreshold.FromPayload(messageType, GetPayload());
        }
    }

    /// <summary>
    /// Represents an operator that creates a timestamped message payload
    /// that threshold value to release the lick detection state. Values above this threshold will untrigger a detected lick.
    /// </summary>
    [DisplayName("TimestampedChannel0UntriggerThresholdPayload")]
    [Description("Creates a timestamped message payload that threshold value to release the lick detection state. Values above this threshold will untrigger a detected lick.")]
    public partial class CreateTimestampedChannel0UntriggerThresholdPayload : CreateChannel0UntriggerThresholdPayload
    {
        /// <summary>
        /// Creates a timestamped message that threshold value to release the lick detection state. Values above this threshold will untrigger a detected lick.
        /// </summary>
        /// <param name="timestamp">The timestamp of the message payload, in seconds.</param>
        /// <param name="messageType">Specifies the type of the created message.</param>
        /// <returns>A new timestamped message for the Channel0UntriggerThreshold register.</returns>
        public HarpMessage GetMessage(double timestamp, MessageType messageType)
        {
            return AllenNeuralDynamics.LicketySplitLickDetector.Channel0UntriggerThreshold.FromPayload(timestamp, messageType, GetPayload());
        }
    }

    /// <summary>
    /// The channel of the lick detector.
    /// </summary>
    [Flags]
    public enum LickChannels : byte
    {
        None = 0x0,
        Channel0 = 0x1,
        Channel1 = 0x2
    }
}
