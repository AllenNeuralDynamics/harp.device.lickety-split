using Bonsai;
using Bonsai.Harp;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Reactive.Linq;
using System.Xml.Serialization;

namespace Aind.LickDetector
{
    /// <summary>
    /// Generates events and processes commands for the LickDetector device connected
    /// at the specified serial port.
    /// </summary>
    [Combinator(MethodName = nameof(Generate))]
    [WorkflowElementCategory(ElementCategory.Source)]
    [Description("Generates events and processes commands for the LickDetector device.")]
    public partial class Device : Bonsai.Harp.Device, INamedElement
    {
        /// <summary>
        /// Represents the unique identity class of the <see cref="LickDetector"/> device.
        /// This field is constant.
        /// </summary>
        public const int WhoAmI = 0;

        /// <summary>
        /// Initializes a new instance of the <see cref="Device"/> class.
        /// </summary>
        public Device() : base(WhoAmI) { }

        string INamedElement.Name => nameof(LickDetector);

        /// <summary>
        /// Gets a read-only mapping from address to register type.
        /// </summary>
        public static new IReadOnlyDictionary<int, Type> RegisterMap { get; } = new Dictionary<int, Type>
            (Bonsai.Harp.Device.RegisterMap.ToDictionary(entry => entry.Key, entry => entry.Value))
        {
            { 32, typeof(DetectionThreshold) }
        };
    }

    /// <summary>
    /// Represents an operator that groups the sequence of <see cref="LickDetector"/>" messages by register type.
    /// </summary>
    [Description("Groups the sequence of LickDetector messages by register type.")]
    public partial class GroupByRegister : Combinator<HarpMessage, IGroupedObservable<Type, HarpMessage>>
    {
        /// <summary>
        /// Groups an observable sequence of <see cref="LickDetector"/> messages
        /// by register type.
        /// </summary>
        /// <param name="source">The sequence of Harp device messages.</param>
        /// <returns>
        /// A sequence of observable groups, each of which corresponds to a unique
        /// <see cref="LickDetector"/> register.
        /// </returns>
        public override IObservable<IGroupedObservable<Type, HarpMessage>> Process(IObservable<HarpMessage> source)
        {
            return source.GroupBy(message => Device.RegisterMap[message.Address]);
        }
    }

    /// <summary>
    /// Represents an operator that filters register-specific messages
    /// reported by the <see cref="LickDetector"/> device.
    /// </summary>
    /// <seealso cref="DetectionThreshold"/>
    [XmlInclude(typeof(DetectionThreshold))]
    [Description("Filters register-specific messages reported by the LickDetector device.")]
    public class FilterRegister : FilterRegisterBuilder, INamedElement
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="FilterRegister"/> class.
        /// </summary>
        public FilterRegister()
        {
            Register = new DetectionThreshold();
        }

        string INamedElement.Name
        {
            get => $"{nameof(LickDetector)}.{GetElementDisplayName(Register)}";
        }
    }

    /// <summary>
    /// Represents an operator which filters and selects specific messages
    /// reported by the LickDetector device.
    /// </summary>
    /// <seealso cref="DetectionThreshold"/>
    [XmlInclude(typeof(DetectionThreshold))]
    [XmlInclude(typeof(TimestampedDetectionThreshold))]
    [Description("Filters and selects specific messages reported by the LickDetector device.")]
    public partial class Parse : ParseBuilder, INamedElement
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="Parse"/> class.
        /// </summary>
        public Parse()
        {
            Register = new DetectionThreshold();
        }

        string INamedElement.Name => $"{nameof(LickDetector)}.{GetElementDisplayName(Register)}";
    }

    /// <summary>
    /// Represents an operator which formats a sequence of values as specific
    /// LickDetector register messages.
    /// </summary>
    /// <seealso cref="DetectionThreshold"/>
    [XmlInclude(typeof(DetectionThreshold))]
    [Description("Formats a sequence of values as specific LickDetector register messages.")]
    public partial class Format : FormatBuilder, INamedElement
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="Format"/> class.
        /// </summary>
        public Format()
        {
            Register = new DetectionThreshold();
        }

        string INamedElement.Name => $"{nameof(LickDetector)}.{GetElementDisplayName(Register)}";
    }

    /// <summary>
    /// Represents a register that manipulates messages from register DetectionThreshold.
    /// </summary>
    [Description("")]
    public partial class DetectionThreshold
    {
        /// <summary>
        /// Represents the address of the <see cref="DetectionThreshold"/> register. This field is constant.
        /// </summary>
        public const int Address = 32;

        /// <summary>
        /// Represents the payload type of the <see cref="DetectionThreshold"/> register. This field is constant.
        /// </summary>
        public const PayloadType RegisterType = PayloadType.U8;

        /// <summary>
        /// Represents the length of the <see cref="DetectionThreshold"/> register. This field is constant.
        /// </summary>
        public const int RegisterLength = 1;

        /// <summary>
        /// Returns the payload data for <see cref="DetectionThreshold"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the message payload.</returns>
        public static byte GetPayload(HarpMessage message)
        {
            return message.GetPayloadByte();
        }

        /// <summary>
        /// Returns the timestamped payload data for <see cref="DetectionThreshold"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the timestamped message payload.</returns>
        public static Timestamped<byte> GetTimestampedPayload(HarpMessage message)
        {
            return message.GetTimestampedPayloadByte();
        }

        /// <summary>
        /// Returns a Harp message for the <see cref="DetectionThreshold"/> register.
        /// </summary>
        /// <param name="messageType">The type of the Harp message.</param>
        /// <param name="value">The value to be stored in the message payload.</param>
        /// <returns>
        /// A <see cref="HarpMessage"/> object for the <see cref="DetectionThreshold"/> register
        /// with the specified message type and payload.
        /// </returns>
        public static HarpMessage FromPayload(MessageType messageType, byte value)
        {
            return HarpMessage.FromByte(Address, messageType, value);
        }

        /// <summary>
        /// Returns a timestamped Harp message for the <see cref="DetectionThreshold"/>
        /// register.
        /// </summary>
        /// <param name="timestamp">The timestamp of the message payload, in seconds.</param>
        /// <param name="messageType">The type of the Harp message.</param>
        /// <param name="value">The value to be stored in the message payload.</param>
        /// <returns>
        /// A <see cref="HarpMessage"/> object for the <see cref="DetectionThreshold"/> register
        /// with the specified message type, timestamp, and payload.
        /// </returns>
        public static HarpMessage FromPayload(double timestamp, MessageType messageType, byte value)
        {
            return HarpMessage.FromByte(Address, timestamp, messageType, value);
        }
    }

    /// <summary>
    /// Provides methods for manipulating timestamped messages from the
    /// DetectionThreshold register.
    /// </summary>
    /// <seealso cref="DetectionThreshold"/>
    [Description("Filters and selects timestamped messages from the DetectionThreshold register.")]
    public partial class TimestampedDetectionThreshold
    {
        /// <summary>
        /// Represents the address of the <see cref="DetectionThreshold"/> register. This field is constant.
        /// </summary>
        public const int Address = DetectionThreshold.Address;

        /// <summary>
        /// Returns timestamped payload data for <see cref="DetectionThreshold"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the timestamped message payload.</returns>
        public static Timestamped<byte> GetPayload(HarpMessage message)
        {
            return DetectionThreshold.GetTimestampedPayload(message);
        }
    }

    /// <summary>
    /// Represents an operator which creates standard message payloads for the
    /// LickDetector device.
    /// </summary>
    /// <seealso cref="CreateDetectionThresholdPayload"/>
    [XmlInclude(typeof(CreateDetectionThresholdPayload))]
    [XmlInclude(typeof(CreateTimestampedDetectionThresholdPayload))]
    [Description("Creates standard message payloads for the LickDetector device.")]
    public partial class CreateMessage : CreateMessageBuilder, INamedElement
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="CreateMessage"/> class.
        /// </summary>
        public CreateMessage()
        {
            Payload = new CreateDetectionThresholdPayload();
        }

        string INamedElement.Name => $"{nameof(LickDetector)}.{GetElementDisplayName(Payload)}";
    }

    /// <summary>
    /// Represents an operator that creates a message payload
    /// for register DetectionThreshold.
    /// </summary>
    [DisplayName("DetectionThresholdPayload")]
    [Description("Creates a message payload for register DetectionThreshold.")]
    public partial class CreateDetectionThresholdPayload
    {
        /// <summary>
        /// Gets or sets the value for register DetectionThreshold.
        /// </summary>
        [Description("The value for register DetectionThreshold.")]
        public byte DetectionThreshold { get; set; }

        /// <summary>
        /// Creates a message payload for the DetectionThreshold register.
        /// </summary>
        /// <returns>The created message payload value.</returns>
        public byte GetPayload()
        {
            return DetectionThreshold;
        }

        /// <summary>
        /// Creates a message for register DetectionThreshold.
        /// </summary>
        /// <param name="messageType">Specifies the type of the created message.</param>
        /// <returns>A new message for the DetectionThreshold register.</returns>
        public HarpMessage GetMessage(MessageType messageType)
        {
            return Aind.LickDetector.DetectionThreshold.FromPayload(messageType, GetPayload());
        }
    }

    /// <summary>
    /// Represents an operator that creates a timestamped message payload
    /// for register DetectionThreshold.
    /// </summary>
    [DisplayName("TimestampedDetectionThresholdPayload")]
    [Description("Creates a timestamped message payload for register DetectionThreshold.")]
    public partial class CreateTimestampedDetectionThresholdPayload : CreateDetectionThresholdPayload
    {
        /// <summary>
        /// Creates a timestamped message for register DetectionThreshold.
        /// </summary>
        /// <param name="timestamp">The timestamp of the message payload, in seconds.</param>
        /// <param name="messageType">Specifies the type of the created message.</param>
        /// <returns>A new timestamped message for the DetectionThreshold register.</returns>
        public HarpMessage GetMessage(double timestamp, MessageType messageType)
        {
            return Aind.LickDetector.DetectionThreshold.FromPayload(timestamp, messageType, GetPayload());
        }
    }
}
